/*
Imagicle print2fax
Copyright (C) 2021 Lorenzo Monti

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "stdafx.h"
#include <windows.h>
#include <bcrypt.h>
#include <string>

#include "Utils.h"

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
//---------------------------------------------------------------------------

static std::wstring BinToHex(PBYTE pbData, DWORD cbData)
{
	std::wstring ret;

	ret.reserve(static_cast<size_t>(cbData) * 2);

	for (DWORD n = 0; n < cbData; n++)
	{
		BYTE b1, b2;
		WCHAR c1, c2;

		b1 = (pbData[n] & 0xF0) >> 4;
		b2 = (pbData[n] & 0x0F);

		if (b1 <= 9)
			c1 = L'0' + b1;
		else
			c1 = L'A' + b1 - 10;

		if (b2 <= 9)
			c2 = L'0' + b2;
		else
			c2 = L'A' + b2 - 10;

		ret.push_back(c1);
		ret.push_back(c2);
	}

	return ret;
}
//---------------------------------------------------------------------------

static void HexToBin(const std::wstring& aString, PBYTE pbData, DWORD cbData)
{
	std::wstring::const_iterator it;
	DWORD cnt = 0, idxb = 0, idxd = 0;
	BYTE b[2];

	for (it = aString.begin(); it != aString.end() && idxd < cbData; it++, cnt++)
	{
		idxb = cnt % 2;

		if (*it >= L'0' && *it <= L'9')
		{
			b[idxb] = *it - L'0';
		}
		else if (*it >= L'a' && *it <= L'f')
		{
			b[idxb] = *it - L'a' + 10;
		}
		else if (*it >= L'A' && *it <= L'F')
		{
			b[idxb] = *it - L'A' + 10;
		}
		else
		{
			break;
		}

		if (idxb == 1)
		{
			pbData[idxd++] = (b[0] << 4) | b[1];
		}
	}
}
//---------------------------------------------------------------------------

static const BYTE rgbAES128Key[] =
{
	0x99, 0x6c, 0xf4, 0xd2, 0x6c, 0x29, 0xba, 0xad,
	0x8b, 0x66, 0x5a, 0x77, 0x94, 0x18, 0xc0, 0x67
};
//---------------------------------------------------------------------------

std::wstring EncryptString(const std::wstring& aString)
{
	BCRYPT_ALG_HANDLE hAesAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	DWORD cbCipherText = 0,
		cbPlainText = 0,
		cbData = 0,
		cbKeyObject = 0,
		cbBlockLen = 0;
	PBYTE pbCipherText = NULL,
		pbKeyObject = NULL,
		pbIV = NULL;
	std::wstring ret, iv, data;

	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&hAesAlg,
		BCRYPT_AES_ALGORITHM,
		NULL,
		0)))
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAesAlg,
		BCRYPT_OBJECT_LENGTH,
		reinterpret_cast<PBYTE>(&cbKeyObject),
		sizeof(DWORD),
		&cbData,
		0)))
	{
		goto LExit;
	}

	pbKeyObject = new BYTE[cbKeyObject];

	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAesAlg,
		BCRYPT_BLOCK_LENGTH,
		reinterpret_cast<PBYTE>(&cbBlockLen),
		sizeof(DWORD),
		&cbData,
		0)))
	{
		goto LExit;
	}

	if (cbBlockLen > 16)
	{
		goto LExit;
	}

	pbIV = new BYTE[16];

	for (DWORD n = 0; n < 16; n++)
		pbIV[n] = rand() % 256;

	iv = BinToHex(pbIV, 16);

	if (!NT_SUCCESS(status = BCryptSetProperty(
		hAesAlg,
		BCRYPT_CHAINING_MODE,
		reinterpret_cast<PBYTE>(BCRYPT_CHAIN_MODE_CBC),
		sizeof(BCRYPT_CHAIN_MODE_CBC),
		0)))
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(
		hAesAlg,
		&hKey,
		pbKeyObject,
		cbKeyObject,
		(PBYTE)rgbAES128Key,
		sizeof(rgbAES128Key),
		0)))
	{
		goto LExit;
	}

	cbPlainText = static_cast<DWORD>(aString.length()) * sizeof(wchar_t);

	if (!NT_SUCCESS(status = BCryptEncrypt(
		hKey,
		(PBYTE)&aString[0],
		cbPlainText,
		NULL,
		pbIV,
		cbBlockLen,
		NULL,
		0,
		&cbCipherText,
		BCRYPT_BLOCK_PADDING)))
	{
		goto LExit;
	}

	pbCipherText = new BYTE[cbCipherText];

	if (!NT_SUCCESS(status = BCryptEncrypt(
		hKey,
		(PBYTE)&aString[0],
		cbPlainText,
		NULL,
		pbIV,
		cbBlockLen,
		pbCipherText,
		cbCipherText,
		&cbData,
		BCRYPT_BLOCK_PADDING)))
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptDestroyKey(hKey)))
	{
		goto LExit;
	}

	data = BinToHex(pbCipherText, cbCipherText);

	ret = iv + data;

LExit:
	if (hAesAlg) BCryptCloseAlgorithmProvider(hAesAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pbCipherText) delete[] pbCipherText;
	if (pbKeyObject) delete[] pbKeyObject;
	if (pbIV) delete[] pbIV;

	return ret;
}
//---------------------------------------------------------------------------

std::wstring DecryptString(const std::wstring& aString)
{
	BCRYPT_ALG_HANDLE hAesAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	DWORD cbCipherText = 0,
		cbPlainText = 0,
		cbData = 0,
		cbKeyObject = 0,
		cbBlockLen = 0,
		cbBlob = 0;
	PBYTE pbCipherText = NULL,
		pbKeyObject = NULL,
		pbBlob = NULL;
	std::wstring ret, data;

	cbBlob = static_cast<DWORD>(aString.length()) / 2;

	if (cbBlob <= 16)
		return L"";

	pbBlob = new BYTE[cbBlob];

	HexToBin(aString, pbBlob, cbBlob);

	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&hAesAlg,
		BCRYPT_AES_ALGORITHM,
		NULL,
		0)))
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAesAlg,
		BCRYPT_OBJECT_LENGTH,
		reinterpret_cast<PBYTE>(&cbKeyObject),
		sizeof(DWORD),
		&cbData,
		0)))
	{
		goto LExit;
	}

	pbKeyObject = new BYTE[cbKeyObject];

	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAesAlg,
		BCRYPT_BLOCK_LENGTH,
		reinterpret_cast<PBYTE>(&cbBlockLen),
		sizeof(DWORD),
		&cbData,
		0)))
	{
		goto LExit;
	}

	if (cbBlockLen > 16)
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptSetProperty(
		hAesAlg,
		BCRYPT_CHAINING_MODE,
		reinterpret_cast<PBYTE>(BCRYPT_CHAIN_MODE_CBC),
		sizeof(BCRYPT_CHAIN_MODE_CBC),
		0)))
	{
		goto LExit;
	}

	if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(
		hAesAlg,
		&hKey,
		pbKeyObject,
		cbKeyObject,
		(PBYTE)rgbAES128Key,
		sizeof(rgbAES128Key),
		0)))
	{
		goto LExit;
	}

	pbCipherText = pbBlob + 16;
	cbCipherText = cbBlob - 16;

	if (!NT_SUCCESS(status = BCryptDecrypt(
		hKey,
		pbCipherText,
		cbCipherText,
		NULL,
		pbBlob,
		cbBlockLen,
		NULL,
		0,
		&cbPlainText,
		BCRYPT_BLOCK_PADDING)))
	{
		goto LExit;
	}

	data.resize(cbPlainText / sizeof(wchar_t));

	if (!NT_SUCCESS(status = BCryptDecrypt(
		hKey,
		pbCipherText,
		cbCipherText,
		NULL,
		pbBlob,
		cbBlockLen,
		reinterpret_cast<PBYTE>(&data[0]),
		cbPlainText,
		&cbPlainText,
		BCRYPT_BLOCK_PADDING)))
	{
		goto LExit;
	}

	data.resize(cbPlainText / sizeof(wchar_t));

	if (!NT_SUCCESS(status = BCryptDestroyKey(hKey)))
	{
		goto LExit;
	}

LExit:
	if (hAesAlg) BCryptCloseAlgorithmProvider(hAesAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pbKeyObject) delete[] pbKeyObject;
	if (pbBlob) delete[] pbBlob;

	return data;
}
//---------------------------------------------------------------------------

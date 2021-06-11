; Imagicle print2fax
; Copyright (C) 2021 Lorenzo Monti
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 3
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#define SrcApp "..\..\wfigui\Release\wfigui.exe"
#define FileVerStr GetFileVersion(SrcApp)
#define StripBuild(str VerStr) Copy(VerStr, 1, RPos(".", VerStr) - 1)
#define AppVerStr StripBuild(FileVerStr)
#define AppName "Imagicle print2fax"

[Setup]
AppId={{2319E902-9BF9-4148-87A5-FA29058A67D0}
AppName={#AppName}
AppVersion={#AppVerStr}
AppVerName={#AppName} {#AppVerStr}
AppPublisher=Imagicle Spa
AppPublisherURL=https://www.imagicle.com/
AppSupportURL=https://www.imagicle.com/
AppUpdatesURL=https://www.imagicle.com/
UninstallDisplayName={#AppName} {#AppVerStr}
UninstallDisplayIcon={app}\bin\wfigui.exe
VersionInfoCompany=Imagicle Spa
VersionInfoCopyright=Copyright © 2021 Lorenzo Monti
VersionInfoDescription={#AppName} setup program
VersionInfoProductName={#AppName}
VersionInfoVersion={#FileVerStr}
WizardImageFile=..\..\..\img\setup.bmp
DisableWelcomePage=no

CreateAppDir=yes
DefaultDirName={pf32}\Imagicle print2fax
DefaultGroupName=Imagicle print2fax

; we take care of these on our own
CloseApplications=no
RestartApplications=no

OutputBaseFilename=wfi-setup
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64

LicenseFile=gpl-3.0.txt

SignTool=lorenzomonti /d "{#AppName}"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"

[CustomMessages]
;en
en.errRegister=Error in port monitor registration!
en.errDeregister=Error in port monitor deregistration!
en.RemPrinter=Removing existing Imagicle print2fax printer...
en.AddPrinter=Adding Imagicle print2fax printer...
en.StartSpooler=Starting Spooler...
en.InstallMsvcrt=Installing Microsoft Visual C++ 2019 redistributable...
en.RegisteringMonitor=Registering port monitor...
en.InstallVirtualPrinter=Install Imagicle print2fax virtual printer
;it
it.errRegister=Errore nella registrazione del port monitor!
it.errDeregister=Errore nella deregistrazione del port monitor!
it.RemPrinter=Rimozione di stampanti Imagicle print2fax esistenti...
it.AddPrinter=Aggiunta della stampante Imagicle print2fax...
it.StartSpooler=Avvio dello Spooler...
it.InstallMsvcrt=Installazione del Microsoft Visual C++ 2019 redistributable...
it.RegisteringMonitor=Registrazione del port monitor...
it.InstallVirtualPrinter=Installa stampante virtuale Imagicle print2fax

[Files]
; x64 files
Source: "..\..\wfimon\x64\Release\wfimon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Check: Is_x64
Source: "..\..\wfimon\x64\Release\wfimonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Check: Is_x64
Source: "..\..\wfimon\x64\Release\regmon.exe"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion; Check: Is_x64
;Source: "..\..\msvc\vc_redist.x64.exe"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion; Check: Is_x64

; x86 files
Source: "..\..\wfimon\Win32\Release\wfimon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Check: Is_x86
Source: "..\..\wfimon\Win32\Release\wfimonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Check: Is_x86
Source: "..\..\wfimon\Win32\Release\regmon.exe"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion; Check: Is_x86
Source: "..\..\msvc\vc_redist.x86.exe"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion; Check: Is_x86

; files common to both architectures
Source: "..\..\wfigui\Release\wfigui.exe"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion
Source: "..\..\wfigui\Release\icudt68.dll"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion
Source: "..\..\wfigui\Release\icuuc68.dll"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion
Source: "..\..\wfigui\Release\libmagic.dll"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion
Source: "..\..\wfigui\Release\magic.mgc"; DestDir: "{app}\bin"; Flags: promptifolder replacesameversion

; language files
Source: "..\..\wfigui\locale\*.mo"; DestDir: "{app}\locale"; Flags: recursesubdirs

; Ghostscript
Source: "..\..\gs\*"; DestDir: "{app}\gs"; Flags: recursesubdirs promptifolder replacesameversion

[Icons]
Name: "{group}\Imagicle print2fax GUI"; Filename: "{app}\bin\wfigui.exe"; WorkingDir: "{app}\bin"

[Registry]
Root: HKLM; SubKey: "Software\Imagicle print2fax"; ValueType: string; ValueName: "WorkingDir"; ValueData: "{app}\bin"

[Run]
Filename: "{app}\bin\vc_redist.x86.exe"; Parameters: "/install /quiet /norestart"; Flags: RunHidden; StatusMsg: {cm:InstallMsvcrt}
;Filename: "{app}\bin\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; Flags: RunHidden; StatusMsg: {cm:InstallMsvcrt}; Check: Is_x64
Filename: "{sys}\net.exe"; Parameters: "start spooler"; Flags: RunHidden; StatusMsg: {cm:StartSpooler}
Filename: "{app}\bin\regmon.exe"; Parameters: "-r"; Flags: RunHidden; StatusMsg: {cm:RegisteringMonitor}
Filename: "{sys}\rundll32.exe"; Parameters: "PrintUI.dll,PrintUIEntry /dl /n ""Imagicle print2fax"" /q"; StatusMsg: {cm:RemPrinter};
Filename: "{sys}\rundll32.exe"; Parameters: "PrintUI.dll,PrintUIEntry /if /b ""Imagicle print2fax"" /f ""{app}\gs\x64\lib\ghostpdf.inf"" /r ""WFI:"" /m ""Ghostscript PDF"" /z /U /Gw"; StatusMsg: {cm:AddPrinter}; Check: Is_x64
Filename: "{sys}\rundll32.exe"; Parameters: "PrintUI.dll,PrintUIEntry /if /b ""Imagicle print2fax"" /f ""{app}\gs\x86\lib\ghostpdf.inf"" /r ""WFI:"" /m ""Ghostscript PDF"" /z /U /Gw"; StatusMsg: {cm:AddPrinter}; Check: Is_x86

[UninstallRun]
Filename: "{sys}\net.exe"; Parameters: "start spooler"; Flags: RunHidden; StatusMsg: {cm:StartSpooler}

[UninstallDelete]
Type: filesandordirs; Name: "{commonappdata}\Imagicle print2fax\faxtmp"

[Code]
var
  bIsAnUpdate: Boolean;
  lblMessage: TLabel;

{----------------------------------------------------------------------------------------}
function Is_x86: Boolean;
begin
  Result := (ProcessorArchitecture = paX86);
end;

{----------------------------------------------------------------------------------------}
function Is_x64: Boolean;
begin
  Result := (ProcessorArchitecture = paX64);
end;

{----------------------------------------------------------------------------------------}
function DestinationFilesExist: Boolean;
begin
  Result := FileExists(ExpandConstant('{sys}\wfimon.dll')) and
            FileExists(ExpandConstant('{sys}\wfimonUI.dll'));
end;

{----------------------------------------------------------------------------------------}
function InitializeSetup: Boolean;
begin
  bIsAnUpdate := DestinationFilesExist;
  Result := True;
end;

{----------------------------------------------------------------------------------------}
procedure CurStepChanged(CurStep: TSetupStep);
var
  rc: Integer;
begin
  case CurStep of
    ssInstall:
      begin
        //stop spooler since we're going to overwrite DLLs in use
        if bIsAnUpdate then
          Exec(ExpandConstant('{sys}\net.exe'), 'stop spooler', '', SW_SHOW, ewWaitUntilTerminated, rc);
      end;
  end;
end;

{----------------------------------------------------------------------------------------}
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  rc: Integer;
begin
  case CurUninstallStep of
    usUninstall:
      begin
        //make sure spooler is running
        Exec(ExpandConstant('{sys}\net.exe'), 'start spooler', '', SW_SHOW, ewWaitUntilTerminated, rc);
        //delete printer
        Exec(ExpandConstant('{sys}\rundll32.exe'), 'PrintUI.dll,PrintUIEntry /dl /n "Imagicle print2fax" /q', '', SW_SHOW, ewWaitUntilTerminated, rc);
        //deregister monitor
        Exec(ExpandConstant('{app}\bin\regmon.exe'), '-d', '', SW_SHOW, ewWaitUntilTerminated, rc);
        //stop spooler
        Exec(ExpandConstant('{sys}\net.exe'), 'stop spooler', '', SW_SHOW, ewWaitUntilTerminated, rc);
      end;
  end;
end;

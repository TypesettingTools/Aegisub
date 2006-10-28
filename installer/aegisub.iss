; Copyright (c) 2006, Fredrik Mellbin
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
;   * Redistributions of source code must retain the above copyright notice,
;     this list of conditions and the following disclaimer.
;   * Redistributions in binary form must reproduce the above copyright notice,
;     this list of conditions and the following disclaimer in the documentation
;     and/or other materials provided with the distribution.
;   * Neither the name of the Aegisub Group nor the names of its contributors
;     may be used to endorse or promote products derived from this software
;     without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; -----------------------------------------------------------------------------
;
; AEGISUB
;
; Website: http://aegisub.cellosoft.com
; Contact: mailto:zeratul@cellosoft.com
;

[Setup]
AppName=Aegisub
AppVerName=Aegisub 1.10
AppPublisher=The Aegisub Group
AppPublisherURL=http://www.aegisub.net/
AppSupportURL=http://www.aegisub.net/
AppUpdatesURL=http://www.aegisub.net/
DefaultDirName={pf}\Aegisub
DefaultGroupName=Aegisub
AllowNoIcons=yes
LicenseFile=license.txt
OutputBaseFilename=Aegisub-Installer
Compression=lzma/ultra
SolidCompression=yes
MinVersion=0,5
WizardImageFile=welcome.bmp
WizardSmallImageFile=aegisub2.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "Full"; Description: "Full Installation"
Name: "Custom"; Description: "Custom"; Flags: IsCustom;

[Components]
Name: "APF"; Description: "Aegisub Program Files"; Types: Full; Flags: disablenouninstallwarning
Name: "APF\Aegisub"; Description: "Aegisub"; Types: Full; Flags: disablenouninstallwarning fixed
Name: "APF\Help"; Description: "Aegisub Help"; Types: Full; Flags: disablenouninstallwarning
Name: "APF\VSFilter"; Description: "VSFilter 2.37"; Types: Full; Flags: disablenouninstallwarning

[Files]
;APF\Aegisub
Source: "Aegisub.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Aegisub
Source: "Aegisub Site.url"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Aegisub
Source: "changelog.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Aegisub
Source: "msvcp71.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Aegisub
Source: "msvcr71.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Aegisub
Source: "locale\*"; DestDir: "{app}\locale"; Excludes: "*.svn"; Flags: ignoreversion recursesubdirs createallsubdirs sortfilesbyextension; Components: APF\Aegisub
Source: "automation\*"; DestDir: "{app}\automation"; Excludes: "*.svn"; Attribs: readonly; Flags: ignoreversion recursesubdirs createallsubdirs sortfilesbyextension overwritereadonly uninsremovereadonly ; Components: APF\Aegisub

;APF\Help
Source: "Aegisub.chm"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\Help

;APF\VSFilter
Source: "VSFilter.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: APF\VSFilter

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Icons]
Name: "{group}\Aegisub"; Filename: "{app}\aegisub.exe"; Components: APF\Aegisub
Name: "{group}\{cm:ProgramOnTheWeb,Aegisub}"; Filename: "{app}\Aegisub Site.url"; Components: APF\Aegisub
Name: "{group}\{cm:UninstallProgram,Aegisub}"; Filename: "{uninstallexe}"; Components: APF\Aegisub
Name: "{userdesktop}\Aegisub"; Filename: "{app}\aegisub.exe"; Tasks: desktopicon; Components: APF\Aegisub
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Aegisub"; Filename: "{app}\aegisub.exe"; Tasks: quicklaunchicon; Components: APF\Aegisub

Name: "{group}\Aegisub Help"; Filename: "{app}\Aegisub.chm"; Components: APF\Help

[Run]
Filename: "{app}\aegisub.exe"; Description: "{cm:LaunchProgram,Aegisub}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\autosave"
Type: filesandordirs; Name: "{app}\autoback"

[Code]
function InitializeSetup: Boolean;
var
  InstallDir: string;
  InstalledVersion: string;
  ReturnCode: Integer;
  VersionMS, VersionLS: Cardinal;
begin
  Result := GetVersionNumbers(AddBackslash(ExpandConstant('{sys}')) + 'avisynth.dll', VersionMS , VersionLS);
  if Result then
    Result := VersionMS = 2 shl 16 + 5;

  if not Result then
    if MsgBox('Aegisub requires Avisynth 2.5 to be installed in order to work.'#13#10'The latest stable Avisynth version can be downloaded from from http://sourceforge.net/projects/avisynth2/files'#13#10'Proceed anyway?', mbConfirmation, MB_YESNO) = IDNO then
      Exit;
  
  if VersionLS < 6 shl 16 then
    if MsgBox('An outdated version of Avisynth was found (2.5.' + IntToStr(VersionLS shr 16) + '). Aegisub requires at least Avisynth 2.5.6a to be installed in order to work properly.'#13#10'The latest stable Avisynth version can be downloaded from from http://sourceforge.net/projects/avisynth2/files'#13#10'Proceed anyway?', mbConfirmation, MB_YESNO) = IDNO then
      Exit;

  if RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstallDir', InstallDir) and RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstVer', InstalledVersion) and FileExists(AddBackslash(InstallDir) + 'uninstall.exe') then
  begin
    if not MsgBox('A previous Aegisub install has been detected (Version ' + InstalledVersion + ').'#13#10'Due to changes from the old installer you are strongly encouraged to uninstall it first?', mbConfirmation, MB_YESNO) = IDYES then
      Exit;

    if FileCopy(AddBackslash(InstallDir) + 'uninstall.exe', AddBackslash(ExpandConstant('{tmp}')) + 'aegisub-uninstall.exe', False) then
    begin
      Exec(AddBackslash(ExpandConstant('{tmp}')) + 'aegisub-uninstall.exe', '_?=' + InstallDir, InstallDir, SW_SHOW, ewWaitUntilTerminated, ReturnCode);
      DeleteFile(AddBackslash(ExpandConstant('{tmp}')) + 'aegisub-uninstall.exe');
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  UninstallString: string;
  ReturnCode: Integer;
begin
  case CurStep of
    ssInstall:
      begin
        //uninstall previous version for upgrades if sdame dir was selected
        if RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\Aegisub_is1', 'UninstallString', UninstallString) then
          if AddBackslash(ExtractFilePath(RemoveQuotes(UninstallString))) = AddBackslash(ExpandConstant('{app}')) then
            Exec(RemoveQuotes(UninstallString), '/VERYSILENT /NORESTART', '', SW_SHOW, ewWaitUntilTerminated, ReturnCode);
      end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usUninstall:
      begin
        if DirExists(AddBackslash(ExpandConstant('{app}')) + 'catalog') then
          if not (UninstallSilent or (MsgBox('Would you like to keep your styles catalog?', mbConfirmation, MB_YESNO) = IDYES)) then
            DelTree(AddBackslash(ExpandConstant('{app}')) + 'catalog', True, True, True);
            
        if FileExists(AddBackslash(ExpandConstant('{app}')) + 'config.dat') or FileExists(AddBackslash(ExpandConstant('{app}')) + 'hotkeys.dat') then
          if not (UninstallSilent or (MsgBox('Would you like to save your Aegisub configuration?', mbConfirmation, MB_YESNO) = IDYES)) then
          begin
            DeleteFile(AddBackslash(ExpandConstant('{app}')) + 'config.dat');
            DeleteFile(AddBackslash(ExpandConstant('{app}')) + 'hotkeys.dat');
          end;
      end;
  end;
end;




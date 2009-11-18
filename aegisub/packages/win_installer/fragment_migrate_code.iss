[Code]
(*
; Copyright (c) 2007-2009, Niels Martin Hansen
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
; Website: http://www.aegisub.org/
; Contact: mailto:nielsm@indvikleren.dk
;
*)


var
  HasLegacyVersion: Boolean;
  LegacyStartMenuFolder: string;
  LegacyInstallFolder: string;
  LegacyVersionNumber: string;


function OldStartMenuFolder(Param: string): string;
begin
  if HasLegacyVersion then
    Result := LegacyStartMenuFolder
  else
    Result := ExpandConstant('{group}');
end;
function OldInstallFolder(Param: string): string;
begin
  if HasLegacyVersion then
    Result := LegacyInstallFolder
  else
    Result := ExpandConstant('{app}');
end;


function BoolToStr(x: Boolean): string;
begin
  if x then Result := 'Yes' else Result := 'No';
end;


function InitializeSetupMigration: Boolean;
begin
  LegacyStartMenuFolder := 'Aegisub';
  LegacyInstallFolder := ExpandConstant('{pf}\Aegisub');
  LegacyVersionNumber := '1.x';

  try
    HasLegacyVersion := RegValueExists(HKLM, 'SOFTWARE\Aegisub\info', 'InstVer');
    Log(Format('Legacy version found: %s', [BoolToStr(HasLegacyVersion)]));

    if RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'StartMenuDir', LegacyStartMenuFolder) then
      LegacyStartMenuFolder := ExpandConstant('{userprograms}\') + LegacyStartMenuFolder;
    Log(Format('Legacy version Start menu folder: %s', [LegacyStartMenuFolder]));

    RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstallDir', LegacyInstallFolder);
    Log(Format('Legacy version install folder: %s', [LegacyInstallFolder]));

    RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstVer', LegacyVersionNumber);
  except
    Log('An exception occurred while trying to detect a legacy installation of Aegisub. Assuming there isn''t one.');
    HasLegacyVersion := False;
  end;

  Result := True;
  if HasLegacyVersion then
    Result := SuppressibleMsgBox(Format('A previous installation of Aegisub %s has been detected and will be removed along with its configuration.'#13#10#13#10'Continue installing and remove old version?', [LegacyVersionNumber]), mbConfirmation, MB_YESNO, IDYES) = IDYES;
end;


procedure MigrateStyleCatalogs;
var
  OldCatalogDir: string;
  NewCatalogDir: string;
  search: TFindRec;
begin
  try
    // Upgrade an 1.x style-catalog by moving it to {appdata}
    OldCatalogDir := OldInstallFolder('') + '\Catalog\';
    Log('-- Migrate style catalogs --');
    if DirExists(OldCatalogDir) then
    begin
      NewCatalogDir := ExpandConstant('{userappdata}\Aegisub\catalog\');
      ForceDirectories(NewCatalogDir);
      Log('Old style catalog dir: ' + OldCatalogDir);
      Log('New catalog dir: ' + NewCatalogDir);
      if FindFirst(OldCatalogDir + '*', search) then
      try
        repeat
          Log('Found style catalog: ' + OldCatalogDir + search.Name);
          if FileCopy(OldCatalogDir+search.Name, NewCatalogDir+search.Name, True) then
          begin
            Log('Copied catalog to: ' + NewCatalogDir+search.Name);
            DeleteFile(OldCatalogDir+search.Name);
          end;
        until not FindNext(search);
      finally
        FindClose(search);
        Log('Done migrating styles');
      end;
      RemoveDir(OldCatalogDir);
    end
    else
      Log('No existing style catalog collection found');
  except
    Log('An exception occurred while trying to migrate style catalogues.');
  end;
end;


procedure UninstallLegacyVersion;
var
  page: TOutputProgressWizardPage;
  file_list: TStringList;
  dir_list: TStringList;
  shortcut_list: TStringList;
  locale_list: TStringList;
  itemsdone, totalitems, i: Integer;
  curname: string;
begin
  // Uninstall Aegisub 1.x
  Log('-- Uninstall legacy version --');
  page := CreateOutputProgressPage('Uninstalling old version', 'Your old installation of Aegisub is being removed');
  try
    page.SetText('Preparing list of files', '');
    page.Show;

    Log('Load file lists');
    ExtractTemporaryFile('legacy_shortcutlist.txt');
    ExtractTemporaryFile('legacy_filelist.txt');
    ExtractTemporaryFile('legacy_locales.txt');
    ExtractTemporaryFile('legacy_dirlist.txt');
    shortcut_list := TStringList.Create;
    shortcut_list.LoadFromFile(ExpandConstant('{tmp}\legacy_shortcutlist.txt'));
    file_list := TStringList.Create;
    file_list.LoadFromFile(ExpandConstant('{tmp}\legacy_filelist.txt'));
    locale_list := TStringList.Create;
    locale_list.LoadFromFile(ExpandConstant('{tmp}\legacy_locales.txt'));
    dir_list := TStringList.Create;
    dir_list.LoadFromFile(ExpandConstant('{tmp}\legacy_dirlist.txt'));
    itemsdone := 0;
    totalitems := file_list.Count + dir_list.Count + shortcut_list.Count + locale_list.Count + 3;
    // Two extra for the registry keys and one for Start menu folder

    for i := 0 to shortcut_list.Count-1 do
    begin
      try
        curname := LegacyStartMenuFolder + '\' + shortcut_list.Strings[i];
        // Check that the link points to somewhere in the dir we're installing to so we don't
        // delete a shortcut to something else
        page.SetText('Removing shortcuts', curname);
        if Pos(LegacyInstallFolder, RemoveQuotes(ReadShellLink(curname))) = 1 then
        begin
          page.SetProgress(itemsdone, totalitems);
          Log('Remove shortcut: ' + curname);
          if not DeleteFile(curname) then Log('* Deletion failed');
          itemsdone := itemsdone + 1;
        end
        else
          Log('Not deleting shortcut, not to old installation: ' + curname);
      except
        Log('Exception removing shortcut: ' + curname);
      end;
    end;
    page.SetText('Removing Start menu folder', LegacyStartMenuFolder);
    page.SetProgress(itemsdone, totalitems);
    Log('Remove directory: ' + LegacyStartMenuFolder);
    if not RemoveDir(LegacyStartMenuFolder) then Log('* Directory deletion failed');
    itemsdone := itemsdone + 1;

    for i := 0 to file_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + file_list.Strings[i];
      page.SetText('Removing files', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove file: ' + curname);
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to locale_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + locale_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory recursively: ' + curname);
      if not DelTree(curname, True, True, True) then Log('* Tree deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to dir_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + dir_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory: ' + curname);
      if not RemoveDir(curname) then Log('* Directory deletion failed');
      itemsdone := itemsdone + 1;
    end;

    page.SetText('Removing registry entries', 'Installation data');
    page.SetProgress(itemsdone, totalitems);
    curname := 'SOFTWARE\Aegisub';
    Log('Remove reg key: HKLM\' + curname);
    if not RegDeleteKeyIncludingSubkeys(HKLM, curname) then Log('* Failed recursively deleting key');

    page.SetText('Removing registry entries', 'Uninstaller entry');
    page.SetProgress(itemsdone+1, totalitems);
    curname := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aegisub ' + LegacyVersionNumber;
    Log('Remove reg key: HKLM\' + curname);
    if not RegDeleteKeyIncludingSubkeys(HKLM, curname) then Log('* Failed recursively deleting key');

    page.SetText('Uninstallation complete', '');
    page.SetProgress(totalitems, totalitems);

  except
    Log('An exception occurred while trying to uninstall a legacy version');
	
  finally
    shortcut_list.Free;
    file_list.Free;
    locale_list.Free;
    dir_list.Free;
    page.Hide;
  end;
end;


function SetFileAttributes(lpFileName: WideString; dwFileAttributes: Longint): Boolean;
external 'SetFileAttributesW@kernel32.dll stdcall';

procedure CleanUpOldVersion;
var
  StartMenuFolder, InstallFolder: string;
  page: TOutputProgressWizardPage;
  file_list: TStringList;
  dir_list: TStringList;
  shortcut_list: TStringList;
  locale_list: TStringList;
  itemsdone, totalitems, i: Integer;
  curname, linktarget: string;
begin
  // Clean up from previous Aegisub 2.x installs
  Log('-- Clean up old versions --');
  page := CreateOutputProgressPage('Cleaning old versions', 'Cleaning up from older versions of Aegisub 2');
  try
    page.SetText('Preparing list of files', '');
    page.Show;

    Log('Load file lists');
    ExtractTemporaryFile('old_shortcutlist.txt');
    ExtractTemporaryFile('old_filelist.txt');
    ExtractTemporaryFile('old_locales.txt');
    ExtractTemporaryFile('old_dirlist.txt');
    shortcut_list := TStringList.Create;
    shortcut_list.LoadFromFile(ExpandConstant('{tmp}\old_shortcutlist.txt'));
    file_list := TStringList.Create;
    file_list.LoadFromFile(ExpandConstant('{tmp}\old_filelist.txt'));
    locale_list := TStringList.Create;
    locale_list.LoadFromFile(ExpandConstant('{tmp}\old_locales.txt'));
    dir_list := TStringList.Create;
    dir_list.LoadFromFile(ExpandConstant('{tmp}\old_dirlist.txt'));
    itemsdone := 0;
    totalitems := file_list.Count + dir_list.Count + shortcut_list.Count + locale_list.Count + 2;
    // Two extra:
    // - Start menu folder
    // - Renaming Aegisub.exe to aegiub32.exe

    StartMenuFolder := ExpandConstant('{commonprograms}\Aegisub\');
    InstallFolder := ExpandConstant('{app}\');
    for i := 0 to shortcut_list.Count-1 do
    begin
      try
        curname := StartMenuFolder + '\' + shortcut_list.Strings[i];
        // Check that the link points to somewhere in the dir we're installing to so we don't
        // delete a shortcut to something else
        page.SetText('Removing shortcuts', curname);
        linktarget := ReadShellLink(curname);
        if Pos(InstallFolder, RemoveQuotes(linktarget)) = 1 then
        begin
          page.SetProgress(itemsdone, totalitems);
          Log('Remove shortcut: ' + curname);
          if not DeleteFile(curname) then Log('* Deletion failed');
          itemsdone := itemsdone + 1;
        end
        else
        begin
          Log('Not deleting shortcut, not to old installation: ' + curname);
          Log('Link target: "' + linktarget + '"; Install folder: "' + InstallFolder + '"');
        end;
      except
        Log('Exception removing shortcut: ' + curname);
      end;
    end;
    page.SetText('Removing Start menu folder', StartMenuFolder);
    page.SetProgress(itemsdone, totalitems);
    Log('Remove directory: ' + StartMenuFolder);
    if not RemoveDir(StartMenuFolder) then Log('* Directory deletion failed');
    itemsdone := itemsdone + 1;

    for i := 0 to file_list.Count-1 do
    begin
      curname := InstallFolder + file_list.Strings[i];
      page.SetText('Removing files', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove file: ' + curname);
      SetFileAttributes(curname, 128); // 128 = FILE_ATTRIBUTE_NORMAL
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;

    // This moving is done to help Windows' link tracking code be able to fix shortcuts
    page.SetText('Moving EXE file to new name', 'Aegisub.exe');
    page.SetProgress(itemsdone, totalitems);
    Log('Rename Aegisub.exe to aegisub32.exe');
    RenameFile(InstallFolder+'Aegisub.exe', InstallFolder+'aegisub32.exe');
    itemsdone := itemsdone + 1;

    for i := 0 to locale_list.Count-1 do
    begin
      curname := InstallFolder + locale_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory recursively: ' + curname);
      if not DelTree(curname, True, True, True) then Log('* Tree deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to dir_list.Count-1 do
    begin
      curname := InstallFolder + dir_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory: ' + curname);
      if not RemoveDir(curname) then Log('* Directory deletion failed');
      itemsdone := itemsdone + 1;
    end;

    page.SetText('Uninstallation complete', '');
    page.SetProgress(totalitems, totalitems);

  except
    Log('An exception occurred while trying to clean up an older version');

  finally
    shortcut_list.Free;
    file_list.Free;
    locale_list.Free;
    dir_list.Free;
    page.Hide;
  end;
end;


procedure CurStepChangedMigration(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    if HasLegacyVersion then
    begin
      MigrateStyleCatalogs;
      UninstallLegacyVersion;
    end;
    CleanUpOldVersion;
  end;
end;







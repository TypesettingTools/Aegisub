// aegisub-csri-host runs each subtitle renderer in its own AppContainer, whose
// profile Aegisub deletes when the renderer exits or, if Aegisub crashed, the
// next time it starts. Delete any that are left when uninstalling. This can
// only see the profiles of the user running the uninstaller.

const
  AppContainerMappings = 'Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppContainer\Mappings';
  SandboxProfilePrefix = 'aegisub.csrihost.';

function DeleteAppContainerProfile(pszAppContainerName: WideString): HResult;
external 'DeleteAppContainerProfile@userenv.dll stdcall delayload';

// Is Name one which Aegisub generates: the prefix followed by a decimal pid,
// a hexadecimal creation time and a decimal counter, separated by dots?
function IsSandboxProfileName(Name: String): Boolean;
var
  Part, I: Integer;
  PartLength: Integer;
  C: Char;
begin
  Result := False;
  if Pos(SandboxProfilePrefix, Name) <> 1 then
    Exit;

  Part := 0;
  PartLength := 0;
  for I := Length(SandboxProfilePrefix) + 1 to Length(Name) do
  begin
    C := Name[I];
    if C = '.' then
    begin
      if PartLength = 0 then
        Exit;
      Part := Part + 1;
      PartLength := 0;
    end
    else if ((C >= '0') and (C <= '9')) or ((Part = 1) and (C >= 'a') and (C <= 'f')) then
      PartLength := PartLength + 1
    else
      Exit;
  end;
  Result := (Part = 2) and (PartLength > 0);
end;

procedure DeleteSandboxProfiles;
var
  Mappings: TArrayOfString;
  Moniker: String;
  I: Integer;
begin
  // AppContainers don't exist before Windows 8, and neither does the function
  if GetWindowsVersion < $06020000 then
    Exit;
  if not RegGetSubkeyNames(HKEY_CURRENT_USER, AppContainerMappings, Mappings) then
    Exit;

  for I := 0 to GetArrayLength(Mappings) - 1 do
  begin
    if RegQueryStringValue(HKEY_CURRENT_USER, AppContainerMappings + '\' + Mappings[I], 'Moniker', Moniker) and
       IsSandboxProfileName(Moniker) then
    begin
      Log('Deleting AppContainer profile ' + Moniker);
      DeleteAppContainerProfile(Moniker);
    end;
  end;
end;

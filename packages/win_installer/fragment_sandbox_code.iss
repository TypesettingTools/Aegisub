// aegisub-csri-host runs each subtitle renderer in its own AppContainer, whose
// profile Aegisub deletes when the renderer exits or, if Aegisub crashed, the
// next time it starts. Delete any that are left when uninstalling.

const
  AppContainerMappings = 'Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppContainer\Mappings';
  SandboxProfilePrefix = 'aegisub.csrihost.';

function DeleteAppContainerProfile(pszAppContainerName: WideString): HResult;
external 'DeleteAppContainerProfile@userenv.dll stdcall delayload';

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
       (Pos(SandboxProfilePrefix, Lowercase(Moniker)) = 1) then
    begin
      Log('Deleting AppContainer profile ' + Moniker);
      DeleteAppContainerProfile(Moniker);
    end;
  end;
end;

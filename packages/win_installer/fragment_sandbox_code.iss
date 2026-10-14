// aegisub-csri-host runs subtitle renderers in an AppContainer, one per
// Aegisub process, whose profile Aegisub deletes when the renderers exit or,
// if Aegisub crashed, the next time it starts. Delete any that are left when
// uninstalling. This can only see the profiles of the user running the
// uninstaller.

const
  AppContainerMappings = 'Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppContainer\Mappings';
  SandboxProfilePrefix = 'aegisub.csrihost.';
  PROCESS_QUERY_LIMITED_INFORMATION = $1000;
  STILL_ACTIVE = 259;
  ERROR_INVALID_PARAMETER = 87;

function DeleteAppContainerProfile(pszAppContainerName: WideString): HResult;
external 'DeleteAppContainerProfile@userenv.dll stdcall delayload';
function OpenProcess(dwDesiredAccess: Cardinal; bInheritHandle: Boolean; dwProcessId: Integer): Integer;
external 'OpenProcess@kernel32.dll stdcall';
function GetExitCodeProcess(hProcess: Integer; var lpExitCode: Cardinal): Boolean;
external 'GetExitCodeProcess@kernel32.dll stdcall';
function GetProcessTimes(hProcess: Integer; var lpCreationTime, lpExitTime, lpKernelTime, lpUserTime: TFileTime): Boolean;
external 'GetProcessTimes@kernel32.dll stdcall';
function CloseHandle(hObject: Integer): Boolean;
external 'CloseHandle@kernel32.dll stdcall';

// Parse a profile name which Aegisub generated: the prefix followed by the
// decimal pid and hexadecimal creation time of the Aegisub process which
// created it, separated by a dot
function ParseSandboxProfileName(Name: String; var Pid: Integer; var CreationTime: Int64): Boolean;
var
  Parts: array[0..1] of String;
  Part, I: Integer;
  C: Char;
begin
  Result := False;
  if Pos(SandboxProfilePrefix, Name) <> 1 then
    Exit;

  Part := 0;
  for I := Length(SandboxProfilePrefix) + 1 to Length(Name) do
  begin
    C := Name[I];
    if C = '.' then
    begin
      if (Parts[Part] = '') or (Part = 1) then
        Exit;
      Part := Part + 1;
    end
    else if ((C >= '0') and (C <= '9')) or ((Part = 1) and (C >= 'a') and (C <= 'f')) then
      Parts[Part] := Parts[Part] + C
    else
      Exit;
  end;
  if (Part <> 1) or (Parts[1] = '') then
    Exit;

  Pid := StrToIntDef(Parts[0], -1);
  CreationTime := StrToInt64Def('$' + Parts[1], -1);
  Result := (Pid >= 0) and (CreationTime >= 0);
end;

// Could the Aegisub process which created a profile still be using it? The
// same check as Aegisub does when deleting profiles which were left behind.
function SandboxProfileOwnerRunning(Pid: Integer; CreationTime: Int64): Boolean;
var
  Process: Integer;
  ExitCode: Cardinal;
  Created, Exited, KernelTime, UserTime: TFileTime;
begin
  Process := OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, Pid);
  if Process = 0 then
  begin
    // Any error other than there being no such process means it might be
    Result := DLLGetLastError <> ERROR_INVALID_PARAMETER;
    Exit;
  end;

  try
    if GetExitCodeProcess(Process, ExitCode) and (ExitCode <> STILL_ACTIVE) then
      Result := False
    else if GetProcessTimes(Process, Created, Exited, KernelTime, UserTime) then
      Result := (Int64(Created.dwHighDateTime) * 4294967296 + Created.dwLowDateTime) = CreationTime
    else
      Result := True;
  finally
    CloseHandle(Process);
  end;
end;

procedure DeleteSandboxProfiles;
var
  Mappings: TArrayOfString;
  Moniker: String;
  Pid: Integer;
  CreationTime: Int64;
  I: Integer;
  Res: HResult;
begin
  // AppContainers don't exist before Windows 8, and neither does the function
  if GetWindowsVersion < $06020000 then
    Exit;
  if not RegGetSubkeyNames(HKEY_CURRENT_USER, AppContainerMappings, Mappings) then
    Exit;

  for I := 0 to GetArrayLength(Mappings) - 1 do
  begin
    if RegQueryStringValue(HKEY_CURRENT_USER, AppContainerMappings + '\' + Mappings[I], 'Moniker', Moniker) and
       ParseSandboxProfileName(Moniker, Pid, CreationTime) and
       not SandboxProfileOwnerRunning(Pid, CreationTime) then
    begin
      Log('Deleting AppContainer profile ' + Moniker);
      Res := DeleteAppContainerProfile(Moniker);
      if Res <> 0 then
        Log(Format('Deleting AppContainer profile %s failed: 0x%.8x', [Moniker, Res]));
    end;
  end;
end;

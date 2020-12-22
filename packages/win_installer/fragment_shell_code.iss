[Code]
// IShellLink interface and CLSID taken from sample
const
  MAX_PATH = 260;
  CLSID_ShellLink = '{00021401-0000-0000-C000-000000000046}';

type
  TWin32FileTime = record
    dwLowDateTime: DWord;
    dwHighDateTime: DWord;
  end;

  TWin32FindData = record
    dwFileAttributes: DWord;
    ftCreationTime: TWin32FileTime;
    ftLastAccessTime: TWin32FileTime;
    ftLastWriteTime: TWin32FileTime;
    nFileSizeHigh: DWord;
    nFileSizeLow: DWord;
    dwReserved0: DWord;
    dwReserved1: DWord;
    cFileName: array[0..MAX_PATH-1] of WideChar;
    cAlternateFileName: array[0..13] of WideChar;
  end;

  IShellLink = interface(IUnknown)
    '{000214F9-0000-0000-C000-000000000046}'
    function GetPath(pszFile: string; cchMaxPath: Integer; var pfd: TWin32FindData; fFlags: DWord): HResult;
    procedure Dummy2; // GetIDList(PIDLIST_ABSOLUTE)
    procedure Dummy3; // SetIDList(PCIDLIST_ABSOLUTE)
    function GetDescription(pszName: String; cchMaxName: Integer): HResult;
    function SetDescription(pszName: String): HResult;
    function GetWorkingDirectory(pszDir: String; cchMaxPath: Integer): HResult;
    function SetWorkingDirectory(pszDir: String): HResult;
    function GetArguments(pszArgs: String; cchMaxPath: Integer): HResult;
    function SetArguments(pszArgs: String): HResult;
    function GetHotkey(var pwHotkey: Word): HResult;
    function SetHotkey(wHotkey: Word): HResult;
    function GetShowCmd(out piShowCmd: Integer): HResult;
    function SetShowCmd(iShowCmd: Integer): HResult;
    function GetIconLocation(pszIconPath: String; cchIconPath: Integer;
      out piIcon: Integer): HResult;
    function SetIconLocation(pszIconPath: String; iIcon: Integer): HResult;
    function SetRelativePath(pszPathRel: String; dwReserved: DWORD): HResult;
    function Resolve(Wnd: HWND; fFlags: DWORD): HResult;
    function SetPath(pszFile: String): HResult;
  end;

  IPersist = interface(IUnknown)
    '{0000010C-0000-0000-C000-000000000046}'
    function GetClassID(var classID: TGUID): HResult;
  end;

  IPersistFile = interface(IPersist)
    '{0000010B-0000-0000-C000-000000000046}'
    function IsDirty: HResult;
    function Load(pszFileName: String; dwMode: Longint): HResult;
    function Save(pszFileName: String; fRemember: BOOL): HResult;
    function SaveCompleted(pszFileName: String): HResult;
    function GetCurFile(out pszFileName: String): HResult;
  end;


function ReadShellLink(LinkFileName: string): string;
var
  Unk: IUnknown;
  PF: IPersistFile;
  SL: IShellLink;
  fnd: TWin32FindData;
  linkpath: string;
begin
  try
    Unk := CreateComObject(StringToGuid(CLSID_ShellLink));

    PF := IPersistFile(Unk);
    OleCheck(PF.Load(LinkFileName, 0));

    SL := IShellLink(Unk);
    SetLength(linkpath, MAX_PATH);
    OleCheck(SL.GetPath(linkpath, MAX_PATH, fnd, 0));

    Result := linkpath;
  except
    Log('Error reading shell link "' + LinkFileName + '": ' + GetExceptionMessage);
    Result := '';
  end;
end;

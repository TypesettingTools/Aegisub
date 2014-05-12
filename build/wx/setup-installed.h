#include "wx/version.h"
#include "wx/cpp.h"

#ifdef _DEBUG
#define wxSUFFIX_DEBUG "d"
#define wxSUFFIX d
#define wxCONFIGURATION Debug
#else
#define wxSUFFIX_DEBUG ""
#define wxSUFFIX
#define wxCONFIGURATION Release
#endif

#ifndef _M_X64
#define wxSETUPH_PATH wxCONCAT5(../../lib/Win32/, wxCONFIGURATION, /msw, wxSUFFIX, /wx/setup.h)
#else
#define wxSETUPH_PATH wxCONCAT5(../../lib/x64/, wxCONFIGURATION, /msw, wxSUFFIX, /wx/setup.h)
#endif

#define wxSETUPH_PATH_STR wxSTRINGIZE(wxSETUPH_PATH)
#include wxSETUPH_PATH_STR

#define wxSUFFIX_STR wxSTRINGIZE(wxSUFFIX)
#define wxSHORT_VERSION_STRING wxSTRINGIZE(wxMAJOR_VERSION) wxSTRINGIZE(wxMINOR_VERSION)
#define wxWX_LIB_NAME(name, subname) "wx" name wxSHORT_VERSION_STRING wxSUFFIX_STR subname
#define wxBASE_LIB_NAME(name) wxWX_LIB_NAME("base", "_" name)
#define wxMSW_LIB_NAME(name) wxWX_LIB_NAME("msw", "_" name)
#define wx3RD_PARTY_LIB_NAME(name) "wx" name wxSUFFIX_DEBUG
#define wx3RD_PARTY_LIB_NAME_U(name) "wx" name wxSUFFIX_STR
#pragma comment(lib, wxWX_LIB_NAME("base", ""))
#pragma comment(lib, wxBASE_LIB_NAME("net"))
#pragma comment(lib, wxBASE_LIB_NAME("xml"))
#pragma comment(lib, wx3RD_PARTY_LIB_NAME("expat"))
#pragma comment(lib, wx3RD_PARTY_LIB_NAME("png"))
#pragma comment(lib, wxMSW_LIB_NAME("core"))
#pragma comment(lib, wxMSW_LIB_NAME("adv"))
#pragma comment(lib, wxMSW_LIB_NAME("gl"))
#pragma comment(lib, wxMSW_LIB_NAME("qa"))
#pragma comment(lib, wxMSW_LIB_NAME("stc"))
#pragma comment(lib, wx3RD_PARTY_LIB_NAME("scintilla"))
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "comdlg32")
#pragma comment(lib, "winspool")
#pragma comment(lib, "winmm")
#pragma comment(lib, "shell32")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")
#pragma comment(lib, "uuid")
#pragma comment(lib, "rpcrt4")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "wsock32")

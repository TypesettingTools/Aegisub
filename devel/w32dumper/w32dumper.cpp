#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <ShellAPI.h>

#include <process.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>

#include "resource.h"


std::wstring CanonicalFileName(std::wstring const &fn)
{
	DWORD bufsize = GetLongPathNameW(fn.c_str(), 0, 0);
	wchar_t *fnbuf = (LPWSTR)malloc(sizeof(*fnbuf)*bufsize);
	bufsize = GetLongPathNameW(fn.c_str(), fnbuf, bufsize);
	auto canfn = std::wstring(fnbuf, fnbuf+bufsize);
	free(fnbuf);
	return canfn;
}

std::wstring GetDumpfileFolder()
{
	std::wstring dumpfile_folder;
	wchar_t appdata_folder[MAX_PATH+1] = {0};
	SHGetFolderPathW(0, CSIDL_APPDATA, 0, SHGFP_TYPE_CURRENT, appdata_folder);
	dumpfile_folder = std::wstring(appdata_folder);
	dumpfile_folder += L"\\Aegisub\\";
	if (CreateDirectoryW(dumpfile_folder.c_str(), 0) == 0 && GetLastError() == ERROR_PATH_NOT_FOUND)
		return std::wstring(); // nowhere to write, somehow there is no %appdata%
	dumpfile_folder += L"dumps\\";
	CreateDirectoryW(dumpfile_folder.c_str(), 0);
	return dumpfile_folder;
}

std::wstring IntToWstring(int n)
{
	wchar_t buf[16];
	swprintf_s(buf, L"%d", n);
	return std::wstring(buf);
}

// reinventing the wheel because the C or C++ std libs don't seem to have
// a function that does something as simple as this.
// note: atoi() and family don't do this, they don't really report success/failure.
template <typename UIntType>
bool try_str2uint(wchar_t *s, UIntType &res)
{
	res = 0;
	while (*s != 0)
	{
		if (*s >= L'0' && *s <= L'9')
		{
			res *= 10;
			res += (*s - L'0');
		}
		else
		{
			// invalid character
			return false;
		}
		++s;
	}
	return true;
}


// ideally identical to the one used by aegisub
// but otherwise the "base" part is fixed
// todo: move this to some shared include file
struct AegisubCrashInfo {
	struct {
		EXCEPTION_POINTERS *exception_pointers;
		DWORD exception_thread_id;
		size_t sz; // size of entire containing struct
	} base;
	wchar_t unhandled_cpp_exception_text[1024];
};


// DNM = Dumper Notify Message
#define DNM_COMPLETED    (WM_APP + 0)
#define DNM_ERROR        (WM_APP + 1)
#define DNM_DUMPSTARTED  (WM_APP + 2)
#define DNM_DUMPFINISHED (WM_APP + 3)
#define DNM_INFOMSG      (WM_APP + 4)


// Data being passed to the worker thread
struct DumperThreadData {
	HWND hwndDlg;
	DWORD target_pid;
	INT_PTR target_infoblock;
};

// Miniclass to make sure the dialog gets sent a message when the thread ends,
// regardless of the reason or manner
struct EnsureDialogNotifiedOfThreadCompletion {
	HWND hwnd;
	EnsureDialogNotifiedOfThreadCompletion(HWND hwnd) : hwnd(hwnd) { }
	~EnsureDialogNotifiedOfThreadCompletion()
	{
		SendMessageW(hwnd, DNM_COMPLETED, 0, 0);
	}
	void error(int code, wchar_t const *message)
	{
		SendMessageW(hwnd, DNM_ERROR, (WPARAM)code, (LPARAM)message);
	}
};

struct WindowsHandle {
	HANDLE handle;
	explicit WindowsHandle(HANDLE handle) : handle(handle) { }
	~WindowsHandle() { if (handle != 0) CloseHandle(handle); }
	operator HANDLE() { return handle; }
};

// fake IsWow64Process function for old 32 bit systems (NT 5.1 and earlier miss it)
BOOL WINAPI FakeIsWow64Process(HANDLE hProcess, PBOOL Wow64Process)
{
	*Wow64Process = FALSE;
	return TRUE;
}

// Thread that will actually find and make dumps of Aegisub processes
void __cdecl dumper_thread(void *data)
{
	DumperThreadData *dtd = static_cast<DumperThreadData *>(data);

	EnsureDialogNotifiedOfThreadCompletion completion_notify(dtd->hwndDlg);

	std::wstring aegisub_filename_prefix;
	std::vector<DWORD> process_ids;

	BOOL dumper_proc_is_wow64 = FALSE;
	BOOL (WINAPI * IsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
	{
		HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
		IsWow64Process = (BOOL(WINAPI*)(HANDLE, PBOOL))GetProcAddress(kernel32, "IsWow64Process");
		if (IsWow64Process == 0) IsWow64Process = FakeIsWow64Process;
	}
	IsWow64Process(GetCurrentProcess(), &dumper_proc_is_wow64);

	if (dtd->target_pid == 0)
	{
		SendMessageW(dtd->hwndDlg, DNM_INFOMSG, 0, (LPARAM)L"Searching for active processes...");

		// Find Aegisub's install dir based on where we are located
		DWORD bufsize = MAX_PATH;
		LPWSTR modfnbuf = (LPWSTR)malloc(sizeof(*modfnbuf)*bufsize);
		DWORD modfnlen = GetModuleFileNameW(0, modfnbuf, bufsize);
		if (modfnlen > bufsize)
		{
			bufsize = modfnlen;
			modfnbuf = (LPWSTR)realloc(modfnbuf, sizeof(*modfnbuf)*bufsize);
			modfnlen = GetModuleFileNameW(0, modfnbuf, bufsize);
		}
		aegisub_filename_prefix = CanonicalFileName(std::wstring(modfnbuf, modfnbuf+modfnlen));
		free(modfnbuf);

		// Chomp it at the last backslash and append "aegisub"
		size_t backslash_pos = aegisub_filename_prefix.rfind(L'\\');
		if (backslash_pos == std::wstring::npos)
		{
			completion_notify.error(2, L"Something is wrong with the installation path");
			return;
		}
		aegisub_filename_prefix.erase(backslash_pos+1);


		// Get pids of all processes
		size_t pidlist_size = 128;
		size_t pidlist_count = 0;
		do {
			process_ids.resize(pidlist_size);
			DWORD bytes_returned = 0;
			if (EnumProcesses(&process_ids[0], sizeof(DWORD)*pidlist_size, &bytes_returned) == 0)
			{
				completion_notify.error(4, L"An error occurred trying to enumerate processes on the system");
				return;
			}
			pidlist_count = bytes_returned / sizeof(DWORD);
		} while (pidlist_count == pidlist_size);
		process_ids.resize(pidlist_count);
	}
	else // target_pid given
	{
		SendMessageW(dtd->hwndDlg, DNM_INFOMSG, 0, (LPARAM)L"Searching for crash target...");

		// just add the single PID to the list, most of the magic happens in the dumping loop
		process_ids.push_back(dtd->target_pid);
	}

	// Figure out where we should be writing dump files to
	std::wstring dumpfile_folder = GetDumpfileFolder();
	if (dumpfile_folder.empty())
	{
		completion_notify.error(3, L"Could not access folder for writing dump files to");
		return;
	}

	// Build a string useful for making filenames more unique
	std::wstring timestring;
	{
		time_t t = time(0);
		tm curtime;
		localtime_s(&curtime, &t);
		wchar_t fmttime[20] = {0};
		swprintf_s(fmttime, L"%4d%02d%02d-%02d%02d%02d",
			curtime.tm_year+1900, curtime.tm_mon, curtime.tm_mday,
			curtime.tm_hour, curtime.tm_min, curtime.tm_sec);
		timestring = std::wstring(fmttime);
	}

	// Check each process for being interesting (i.e. probably an Aegisub process)
	const DWORD mypid = GetCurrentProcessId();
	for (auto ppid = process_ids.begin(); ppid != process_ids.end(); ++ppid)
	{
		if (*ppid == mypid)
			continue;

		WindowsHandle proc(OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, *ppid));
		if (proc == 0)
			continue;

		// Get process name
		std::wstring procfn;
		{
			DWORD bufsize = MAX_PATH;
			LPWSTR modfnbuf = (LPWSTR)malloc(sizeof(*modfnbuf)*bufsize);
			DWORD modfnlen = GetModuleFileNameExW(proc, 0, modfnbuf, bufsize);
			if (modfnlen == 0)
			{
				DWORD err = GetLastError();
				continue;
			}
			if (modfnlen > bufsize)
			{
				bufsize = modfnlen;
				modfnbuf = (LPWSTR)realloc(modfnbuf, sizeof(*modfnbuf)*bufsize);
				modfnlen = GetModuleFileNameExW(proc, 0, modfnbuf, bufsize);
			}
			procfn = CanonicalFileName(std::wstring(modfnbuf, modfnbuf+modfnlen));
			free(modfnbuf);
		}

		// Check it's relevant
		if (dtd->target_pid == 0 && procfn.find(aegisub_filename_prefix) != 0)
			continue;

		// Pick a filename to write
		std::wstring procfn_basename;
		{
			// Chop off everything up to and including last backslash
			size_t pos = procfn.rfind(L'\\');
			if (pos != std::wstring::npos)
				procfn_basename = procfn.substr(pos+1);
			else
				procfn_basename = procfn;
		}

		// Tell about our exploits
		SendMessageW(dtd->hwndDlg, DNM_DUMPSTARTED, (WPARAM)*ppid, (LPARAM)procfn_basename.c_str());

		MINIDUMP_EXCEPTION_INFORMATION exception_information = {0};
		MINIDUMP_USER_STREAM_INFORMATION *user_stream_information = 0;
		if (dtd->target_pid != 0)
		{
			// sanity check: we can't easily work with different-bitness processes
			BOOL target_proc_is_wow64 = FALSE;
			IsWow64Process(proc, &target_proc_is_wow64);
			if (target_proc_is_wow64 != dumper_proc_is_wow64)
			{
				SendMessageW(dtd->hwndDlg, DNM_INFOMSG, 0, (LPARAM)(
					target_proc_is_wow64
					? L"Target process is 32 bit, but crash dumper is 64 bit. Cannot include all information in dump."
					: L"Target process is 64 bit, but crash dumper is 32 bit. Cannot include all infromation in dump."
					));
				goto skip_advanced_dump;
			}

			AegisubCrashInfo crashinfo;
			if (ReadProcessMemory(proc, (LPCVOID)dtd->target_infoblock, &crashinfo, sizeof(crashinfo.base), 0) == FALSE ||
				ReadProcessMemory(proc, (LPCVOID)dtd->target_infoblock, &crashinfo, crashinfo.base.sz, 0) == FALSE)
			{
				SendMessageW(dtd->hwndDlg, DNM_INFOMSG, 0, (LPARAM)L"Failed to read detailed crash information. Proceeding with basic dump.");
				goto skip_advanced_dump;
			}

			// fill in exception_pointers stuff
			exception_information.ThreadId = crashinfo.base.exception_thread_id;
			exception_information.ExceptionPointers = crashinfo.base.exception_pointers;
			exception_information.ClientPointers = TRUE;

			// use complex information if available
			if (sizeof(crashinfo) != crashinfo.base.sz)
			{
				SendMessageW(dtd->hwndDlg, DNM_INFOMSG, 0, (LPARAM)L"Detailed crash information is unsupported version, only using exception information (if present).");
			}
			else
			{
				// design pattern: allocate some memory and don't plan to ever free it
				user_stream_information = new MINIDUMP_USER_STREAM_INFORMATION;
				user_stream_information->UserStreamArray = new MINIDUMP_USER_STREAM[1];
				user_stream_information->UserStreamCount = 1;

				user_stream_information->UserStreamArray[0].Type = CommentStreamW;
				user_stream_information->UserStreamArray[0].Buffer = crashinfo.unhandled_cpp_exception_text;
				user_stream_information->UserStreamArray[0].BufferSize = sizeof(crashinfo.unhandled_cpp_exception_text);
			}
		}
skip_advanced_dump:

		std::wstring dumpfile_name =
			dumpfile_folder +
			procfn_basename + L'-' +
			IntToWstring(*ppid) + L'-' +
			timestring +
			L".dmp";

		WindowsHandle dumpfile(CreateFileW(dumpfile_name.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0));

		MiniDumpWriteDump(
			proc,
			*ppid,
			dumpfile,
			MINIDUMP_TYPE(MiniDumpWithThreadInfo|MiniDumpIgnoreInaccessibleMemory|MiniDumpWithIndirectlyReferencedMemory),
			exception_information.ExceptionPointers ? &exception_information : 0,
			user_stream_information,
			0);

		SendMessageW(dtd->hwndDlg, DNM_DUMPFINISHED, 0, (LPARAM)dumpfile_name.c_str());
	}
}


int numdumps = 0;
int numerrors = 0;

void AddStringToListbox(HWND hwndDlg, std::wstring const &str)
{
	SendDlgItemMessageW(hwndDlg, IDC_LOGLIST, LB_ADDSTRING, 0, (LPARAM)str.c_str());
}

INT_PTR CALLBACK dialog_msghandler(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case DNM_COMPLETED:
		EnableWindow(GetDlgItem(hwndDlg, IDCLOSE), TRUE);
		if (numerrors > 0)
			AddStringToListbox(hwndDlg, L"Finished with errors.");
		else if (numdumps > 0)
			AddStringToListbox(hwndDlg, std::wstring(L"Completed ") + IntToWstring(numdumps) + (numdumps>1?L" minidumps.":L" minidump."));
		else
			AddStringToListbox(hwndDlg, L"Finished, found no processes to dump.");
		return TRUE;

	case DNM_ERROR:
		numerrors += 1;
		AddStringToListbox(hwndDlg, std::wstring(L"An error occurred: ") + (wchar_t const *)lParam);
		return TRUE;

	case DNM_DUMPSTARTED:
		numdumps += 1;
		AddStringToListbox(hwndDlg, std::wstring(L"Beginning dump of pid ") + IntToWstring(wParam) + L" (" + (wchar_t const *)lParam + L")");
		return TRUE;

	case DNM_DUMPFINISHED:
		AddStringToListbox(hwndDlg, std::wstring(L"    Finished dump: ") + (wchar_t const *)lParam);
		return TRUE;

	case DNM_INFOMSG:
		AddStringToListbox(hwndDlg, (wchar_t const *)lParam);
		return true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCLOSE && HIWORD(wParam) == BN_CLICKED)
		{
			PostQuitMessage(0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR &nm = *(NMHDR*)lParam;
			if (nm.idFrom == IDC_DUMPFOLDERLINK && (nm.code == NM_CLICK || nm.code == NM_RETURN))
			{
				std::wstring dumpfile_folder = GetDumpfileFolder();
				ShellExecuteW(hwndDlg, L"open", dumpfile_folder.c_str(), 0, 0, SW_SHOWNORMAL);
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}


#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CoInitializeEx(0, COINIT_MULTITHREADED);
	INITCOMMONCONTROLSEX iccx = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_LINK_CLASS|ICC_STANDARD_CLASSES
	};
	if (InitCommonControlsEx(&iccx) == FALSE)
		ExitProcess(1);

	HWND hwndDlg = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_W32DUMPER), 0, dialog_msghandler);
	if (hwndDlg == 0)
		ExitProcess(2);

	// todo: figure out a commandline format and parse it
	// todo: what about exception data? shared memory?

	// idea: SetUnhandledExceptionFilter() in Aegisub.
	// When it hits, fill in a global storage struct with various useful information including
	// pointer to EXCEPTION_POINTERS struct, then launch w32dumper with arguments:
	//    -crash <pid> <address_of_global_struct>
	// Since the size of the struct is known and Aegisub will be OpenProcess()'d with VM_READ
	// privileges anyway, we can ReadProcessMemory() the struct out and parse it for interesting
	// information. The EXCEPTION_POINTERS pointer will be to the address in Aegisub's VM but
	// that's okay, MiniDumpWriteDump() can handle that.
	// Last problem is then to make sure that Aegisub stays in the correct state (with exception
	// pointers valid and all that), maybe just wait for the w32dumper process, then abort? Will
	// other threads continue running then? If so, should they all be suspended?
	//
	// linkdump:
	//  SetUnhandledExceptionFilter: <http://msdn.microsoft.com/en-us/library/windows/desktop/ms680634%28v=vs.85%29.aspx>
	//  ReadProcessMemory: <http://msdn.microsoft.com/en-us/library/windows/desktop/ms680553%28v=vs.85%29.aspx>

	DumperThreadData dtd = { hwndDlg };

	{
		int argc = 0;
		// this will allocate a bit of memory, it's a waste of time to free it since the allocation
		// disappears anyway when the process exits, and this process generally shouldn't be long-lived
		// and this isn't a recurring allocation either.
		wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		// accept command line args "-crash <pid> <infoblock_address>"
		// infoblock_address is given as decimal, for simplicity
		if (argc >= 4 && wcscmp(argv[1], L"-crash") == 0)
		{
			if (try_str2uint(argv[2], dtd.target_pid) && try_str2uint(argv[3], dtd.target_infoblock))
			{
				// well we got some values, the thread will act on them, no more to do here...
			}
			else
			{
				dtd.target_pid = 0;
				dtd.target_infoblock = 0;
			}
		}
	}

	uintptr_t dumper_thread_handle = _beginthread(dumper_thread, 0, &dtd);
	ShowWindow(hwndDlg, SW_SHOWNORMAL);

	MSG msg;
	BOOL gmret;
	while ((gmret = GetMessageW(&msg, 0, 0, 0)) != 0)
	{
		if (gmret == -1)
		{
			ExitProcess(3);
		}
		else if (!IsDialogMessageW(hwndDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	CoUninitialize();

	return gmret;
}

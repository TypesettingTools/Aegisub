local ffi = require("ffi")

if ffi.os ~= "Windows" then
	return
end

-- Safety first!
do
	if pcall(string.dump, io.open) then
		error("io.open is already patched!")
	end

	local function index(t, k)
		return t[k]
	end

	if pcall(index, ffi.C, "GetACP") == false then
		ffi.cdef[[
		uint32_t __stdcall GetACP();
		]]
	end
end

local CP_UTF8 = 65001
local MB_ERR_INVALID_CHARS = 8

if ffi.C.GetACP() == CP_UTF8 then
	-- "Use Unicode UTF-8 for worldwide language support" is ticked.
	-- Don't bother patching it.
	return
end

ffi.cdef[[
int32_t __stdcall MultiByteToWideChar(
	uint32_t CodePage,
	uint32_t dwFlags,
	const char *lpMultiByteStr,
	int32_t cbMultiByte,
	wchar_t *lpWideCharStr,
	int32_t cchWideChar
);
void *_wfreopen(wchar_t *path, wchar_t *mode, void *file);
int32_t _wrename(wchar_t *oldname, wchar_t *newname);
int32_t _wremove(wchar_t *path);
int32_t _wsystem(wchar_t *command);
char *strerror(int errnum);
]]

local function widen(ch)
	local size = ffi.C.MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ch, #ch, nil, 0)
	if size == 0 then
		error(fname .. ": invalid character sequence")
	end

	local buf = ffi.new("wchar_t[?]", size + 1)
	if ffi.C.MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ch, #ch, buf, size) == 0 then
		error(fname .. ": char conversion error")
	end

	return buf
end

local function fileresult(stat, fname)
	if stat == 0 then
		return true
	end

	local errno = ffi.errno()
	local msg = ffi.C.strerror(errno)

	if fname then
		return nil, fname .. ": " .. tostring(msg), errno
	end
	return nil, msg, errno
end

local function execresult(stat)
	if stat == -1 then
		return fileresult(0, nil)
	end

	if stat == 0 then
		return true, "exit", stat
	end
	return nil, "exit", stat
end

local orig_open = io.open
local orig_rename = os.rename
local orig_remove = os.remove
local orig_execute = os.execute

function io.open(fname, mode)
	local wfname = widen(fname)
	if not mode then
		mode = "r"
	end
	local wmode = widen(mode)

	local file = assert(orig_open("nul", "rb"))
	if ffi.C._wfreopen(wfname, wmode, file) == nil then
		local msg, errno = select(2, file:close())
		return nil, fname .. ": " .. msg, errno
	end

	return file
end

function os.rename(oldname, newname)
	local woldname = widen(oldname)
	local wnewname = widen(newname)

	local stat = ffi.C._wrename(woldname, wnewname)
	return fileresult(stat, oldname)
end

function os.remove(fname)
	local wfname = widen(fname)

	local stat = ffi.C._wremove(wfname)
	return fileresult(stat, fname)
end

function os.execute(command)
	local wcommand = command
	if command then
		wcommand = widen(command)
    local stat = ffi.C._wsystem(wcommand)
		return execresult(stat)
	end

	return true
end

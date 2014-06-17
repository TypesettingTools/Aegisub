#!../vendor/luajit/src/host/minilua
-- Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
--
-- Permission to use, copy, modify, and distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--
-- Aegisub Project http://www.aegisub.org/

if not arg or #arg == 0 then arg = {...} end

if #arg ~= 3 then
  io.stdout:write('Usage: <manifest>[in] <c++ file>[out] <header>[out]\n')
  os.exit(1)
end

local function try_open(filename, mode)
  local file, err = io.open(filename, mode)
  if not file then
    io.stdout:write(string.format('Failed to open "%s": %s\n', filename, err))
    os.exit(1)
  end
  return file
end

io.stdout:write("Manifest: " .. arg[1] .. " CPP: " .. arg[2] .. " Header: " .. arg[3] .. '\n')

local manifest = try_open(arg[1], 'r')
local out_cpp = try_open(arg[2], 'w')
local out_h = try_open(arg[3], 'w')

local path = arg[1]:match'(.*/).*' or ''

out_cpp:write('#include "libresrc.h"\n')

for line in manifest:lines() do
  if line:find('.') then
    local file = try_open(path..line, 'rb')
    local id = line:gsub('^.*/', ''):gsub('\.[a-z]+$', '')
    out_cpp:write("const unsigned char " .. id .. "[] = {")

    local len = 0
    while true do
      local bytes = file:read(65536)
      if not bytes then break end

      for i = 1, #bytes do
        if len > 0 then out_cpp:write(',') end
        out_cpp:write(string.format('%d', bytes:byte(i)))
        len = len + 1
      end
    end
    out_cpp:write('};\n')
    out_h:write(string.format('extern const unsigned char %s[%d];\n', id, len))
    file:close()
  end
end

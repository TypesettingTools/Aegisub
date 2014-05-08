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

local ICU_VERSION = 'icudt53'

local function try_open(filename, mode)
  local file, err = io.open(filename, mode)
  if not file then
    io.stdout:write(string.format('Failed to open "%s": %s\n', filename, err))
    os.exit(1)
  end
  return file
end

local icu_root, out_path = ...

local infile = try_open(string.format('%s/data/in/%sl.dat', icu_root, ICU_VERSION), 'rb')
local outfile = try_open(out_path, 'w')

outfile:write("const unsigned char " .. ICU_VERSION .. "_dat[] = {")

local len = 0
while true do
  local bytes = infile:read(65536)
  if not bytes then break end

  for i = 1, #bytes do
    if len > 0 then outfile:write(',') end
    outfile:write(string.format('%d', bytes:byte(i)))
    len = len + 1
  end
end
outfile:write('};\n')


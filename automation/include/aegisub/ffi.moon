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

ffi = require 'ffi'
ffi.cdef[[
  void free(void *ptr);
]]

char_ptr = ffi.typeof 'char *'

-- Convert a const char * to a lua string, returning nil rather than crashing
-- if it's NULL and freeing the input if it's non-const
string = (cdata) ->
  return nil if cdata == nil
  str = ffi.string cdata
  if type(cdata) == char_ptr
    ffi.C.free cdata
  str

err_buff = ffi.new 'char *[1]'

-- Convert a function which has an error out parameter to one which returns
-- the original return value and the error as a string
err_arg_to_multiple_return = (f) -> (arg) ->
  err_buff[0] = nil
  result = if arg != nil
    f arg, err_buff
  else
    f err_buff
  errmsg = string err_buff[0]
  if errmsg
    return nil, errmsg
  return result

{:string, :err_arg_to_multiple_return}

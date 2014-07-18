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

impl = require 'aegisub.__lfs_impl'
ffi = require 'ffi'
ffi_util = require 'aegisub.ffi'

for k, v in pairs impl
  impl[k] = ffi_util.err_arg_to_multiple_return v

string_ret = (f) -> (...) ->
  res, err = f ...
  ffi_util.string(res), err

number_ret = (f) -> (...) ->
  res, err = f ...
  tonumber(res), err

attributes = (path, field) ->
  switch field
    when 'mode'
      res, err = impl.get_mode path
      ffi_util.string(res), err
    when 'modification'
      res, err = impl.get_mtime path
      tonumber(res), err
    when 'size'
      res, err = impl.get_size path
      tonumber(res), err
    else
      mode, err = impl.get_mode path
      if err or mode == nil then return nil, err

      mod, err = impl.get_mtime path
      if err then return nil, err

      size, err = impl.get_size path
      if err then return nil, err

      mode: ffi_util.string(mode), modification: tonumber(mod), size: tonumber(size)

class dir_iter
  new: (iter) =>
    @iter = ffi.gc iter, -> impl.dir_free iter
  close: =>
    impl.dir_close @iter
  next: =>
    str, err = impl.dir_next @iter
    if err then error err, 2
    ffi_util.string str

dir = (path) ->
  obj, err = impl.dir_new path
  if err
    error 2, err
  iter = dir_iter obj
  iter.next, iter

return {
  :attributes
  chdir: number_ret impl.chdir
  currentdir: string_ret impl.currentdir
  :dir
  mkdir: number_ret impl.mkdir
  rmdir: number_ret impl.rmdir
  touch: number_ret impl.touch
}

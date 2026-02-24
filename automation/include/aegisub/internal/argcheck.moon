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

assert   = assert
error    = error
select   = select
tostring = tostring
type     = type

is_type = (v, ty, expected) ->
  ty == expected or (ty == 'table' and v.__class and v.__class.__name == expected)

(argfmt) ->
  assert type(argfmt) == 'string'
  min_args = 0
  max_args = 0
  checks = {}
  for arg in argfmt\gmatch '[^ ]+'
    if arg == '...'
      max_args = nil
      break

    max_args += 1

    optional = arg\sub(1, 1) == '?'
    if optional
      arg = arg\sub 2
    else
      min_args += 1

    if arg\find '|'
      types = [ty for ty in arg\gmatch '[^|]+']
      checks[max_args] = (i, v) ->
        if v == nil
          return if optional
          error "Argument ##{i} should be a #{arg}, is nil", 4
        ty = type v
        for argtype in *types
          return if is_type v, ty, argtype
        error "Argument ##{i} should be a #{arg}, is #{ty} (#{v})", 3
    else
      checks[max_args] = (i, v) ->
        if v == nil
          return if optional
          error "Argument ##{i} should be a #{arg}, is nil", 4
        ty = type v
        return if is_type v, ty, arg
        error "Argument ##{i} should be a #{arg}, is #{ty} (#{v})", 3

  (fn) -> (...) ->
    arg_count = select '#', ...
    if arg_count < min_args or (max_args and arg_count > max_args)
      if min_args == max_args
        error "Expected #{min_args} arguments, got #{arg_count}", 3
      else if max_args
        error "Expected #{min_args}-#{max_args} arguments, got #{arg_count}", 3
      else
        error "Expected at least #{min_args} arguments, got #{arg_count}", 3

    for i=1,arg_count
      if not checks[i] then break
      checks[i] i, select i, ...

    fn ...


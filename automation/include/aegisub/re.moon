-- Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

error  = error
next   = next
select = select
type   = type

bit = require 'bit'
ffi = require 'ffi'
ffi_util = require 'aegisub.ffi'
check = require 'aegisub.argcheck'

ffi.cdef[[
  typedef struct agi_re_flag {
    const char *name;
    int value;
  } agi_re_flag;
]]
regex_flag = ffi.typeof 'agi_re_flag'

-- Get the boost::eegex binding
regex = require 'aegisub.__re_impl'

-- Wrappers to convert returned values from C types to Lua types
search = (re, str, start) ->
  return unless start <= str\len()
  res = regex.search re, str, str\len(), start
  return unless res != nil
  first, last = res[0], res[1]
  ffi.gc(res, ffi.C.free)
  first, last

replace = (re, replacement, str, max_count) ->
  ffi_util.string regex.replace re, replacement, str, str\len(), max_count

match = (re, str, start) ->
  assert start <= str\len()
  m = regex.match re, str, str\len(), start
  return unless m != nil
  ffi.gc m, regex.match_free

get_match = (m, idx) ->
  res = regex.get_match m, idx
  return unless res != nil
  res[0], res[1] -- Result buffer is owned by match so no need to free

err_buff = ffi.new 'char *[1]'
compile = (pattern, flags) ->
  err_buff[0] = nil
  re = regex.compile pattern, flags, err_buff
  if err_buff[0] != nil
    return ffi.string err_buff[0]
  ffi.gc re, regex.regex_free

-- Return the first n elements from ...
select_first = (n, a, ...) ->
  if n == 0 then return
  a, select_first n - 1, ...

-- Bitwise-or together regex flags passed as arguments to a function
process_flags = (...) ->
  flags = 0
  for i = 1, select '#', ...
    v = select i, ...
    if not ffi.istype regex_flag, v
      error 'Flags must follow all non-flag arguments', 3
    flags = bit.bor flags, v.value
  flags

-- Extract the flags from ..., bitwise OR them together, and move them to the
-- front of ...
unpack_args = (...) ->
  flags_start = nil
  for i = 1, select '#', ...
    v = select i, ...
    if ffi.istype regex_flag, v
      flags_start = i
      break

  return 0, ... unless flags_start
  process_flags(select flags_start, ...), select_first flags_start - 1, ...

-- Replace a match with the value returned from func when passed the match
replace_match = (match, func, str, last, acc) ->
  -- Copy everything between the last match and this match
  if last < match.last
    acc[#acc + 1] = str\sub last, match.first - 1

  repl = func match.str, match.first, match.last

  -- If it didn't return a string just leave the old value
  acc[#acc + 1] = if type(repl) == 'string' then repl else match.str

  match.first, match.last + 1

-- Replace all matches from a single iteration of the regexp
do_single_replace_fun = (re, func, str, acc, pos) ->
  matches = re\match str, pos

  -- No more matches so just return what's left of the input
  return pos unless matches

  -- If there's only one match then there's no capturing groups and we need
  -- to pass the entire match to the replace function, but if there's
  -- multiple then we want to skip the full match and only pass the capturing
  -- groups.
  start = if #matches == 1 then 1 else 2
  last = pos
  local first
  for i = start, #matches
    first, last = replace_match matches[i], func, str, last, acc

  -- Always eat at least one character from the input or we'll just make the
  -- same match max_count times
  if first == last
    acc[#acc + 1] = str\sub last, last
    last += 1

  return last, matches[1].first <= str\len()

do_replace_fun = (re, func, str, max) ->
  acc = {}
  pos = 1
  local i
  for i = 1, max do
    pos, more = do_single_replace_fun re, func, str, acc, pos
    unless more
      max = i
      break
  table.concat(acc, '') .. str\sub pos

-- Compiled regular expression type protoype
class RegEx
  -- Verify that a valid value was passed for self
  _check_self: =>
    unless @__class == RegEx
      error 're method called with invalid self. You probably used . when : is needed.', 3

  new: (@_regex, @_level) =>

  gsplit: check'RegEx string ?boolean ?number' (str, skip_empty, max_split) =>
    if not max_split or max_split <= 0 then max_split = str\len()

    start = 0
    prev = 1
    do_split = () ->
      if not str or str\len() == 0 then return

      local first, last
      if max_split > 0
        first, last = search @_regex, str, start

      if not first or first > str\len()
        ret = str\sub prev, str\len()
        str = nil
        return if skip_empty and ret\len() == 0 then nil else ret

      ret = str\sub prev, first - 1
      prev = last + 1

      start = if start >= last then start + 1 else last

      if skip_empty and ret\len() == 0
        do_split()
      else
        max_split -= 1
        ret

    do_split

  split: check'RegEx string ?boolean ?number' (str, skip_empty, max_split) =>
    [v for v in @gsplit str, skip_empty, max_split]

  gfind: check'RegEx string' (str) =>
    start = 0
    ->
      first, last = search(@_regex, str, start)
      return unless first

      start = if last > start then last else start + 1
      str\sub(first, last), first, last

  find: check'RegEx string' (str) =>
    ret = [str: s, first: f, last: l for s, f, l in @gfind(str)]
    next(ret) and ret

  sub: check'RegEx string string|function ?number' (str, repl, max_count) =>
    max_count = str\len() + 1 if not max_count or max_count == 0

    if type(repl) == 'function'
       do_replace_fun @, repl, str, max_count
    elseif type(repl) == 'string'
      replace @_regex, repl, str, max_count

  gmatch: check'RegEx string ?number' (str, start) =>
    start = if start then start - 1 else 0

    m = match @_regex, str, start
    i = 0
    ->
      return unless m
      first, last = get_match m, i
      return unless first
      i += 1

      {
        str: str\sub first + start, last + start
        first: first + start
        last: last + start
      }

  match: check'RegEx string ?number' (str, start) =>
    ret = [v for v in @gmatch str, start]
    -- Return nil rather than a empty table so that if re.match(...) works
    return nil if next(ret) == nil
    ret

-- Create a regex object from a pattern, flags, and error depth
real_compile = (pattern, level, flags, stored_level) ->
  if pattern == ''
    error 'Regular expression must not be empty', level + 1

  re = compile pattern, flags
  if type(re) == 'string'
    error regex, level + 1

  RegEx re, stored_level or level + 1

-- Compile a pattern then invoke a method on it
invoke = (str, pattern, fn, flags, ...) ->
  compiled_regex = real_compile(pattern, 3, flags)
  compiled_regex[fn](compiled_regex, str, ...)

-- Generate a static version of a method with arg type checking
gen_wrapper = (impl_name) -> check'string string ...' (str, pattern, ...) ->
  invoke str, pattern, impl_name, unpack_args ...

-- And now at last the actual public API
do
  re = {
    compile: check'string ...' (pattern, ...) ->
      real_compile pattern, 2, process_flags(...), 2

    split:  gen_wrapper 'split'
    gsplit: gen_wrapper 'gsplit'
    find:   gen_wrapper 'find'
    gfind:  gen_wrapper 'gfind'
    match:  gen_wrapper 'match'
    gmatch: gen_wrapper 'gmatch'
    sub:    gen_wrapper 'sub'
  }

  i = 0
  flags = regex.get_flags()
  while flags[i].name != nil
    re[ffi.string flags[i].name] = flags[i]
    i += 1

  re

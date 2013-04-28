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

-- Get the boost::regex binding
regex = aegisub.__init_regex()

-- Return the first n elements from ...
select_first = (n, a, ...) ->
  if n == 0 then return
  a, select_first n - 1, ...

-- Extract the flags from ..., bitwise OR them together, and move them to the
-- front of ...
unpack_args = (...) ->
  userdata_start = nil
  for i = 1, select '#', ...
    v = select i, ...
    if type(v) == 'userdata'
      userdata_start = i
      break

  return 0, ... unless userdata_start

  flags = regex.process_flags select userdata_start, ...
  if type(flags) == 'string'
    error(flags, 3)

  flags, select_first userdata_start - 1, ...


-- Typecheck a variable and throw an error if it fails
check_arg = (arg, expected_type, argn, func_name, level) ->
  if type(arg) != expected_type
    error "Argument #{argn} to #{func_name} should be a '#{expected_type}', is '#{type(arg)}' (#{arg})",
      level + 1

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

  start = 1
  gsplit: (str, skip_empty, max_split) =>
    @_check_self!
    check_arg str, 'string', 2, 'gsplit', @_level
    if not max_split or max_split <= 0 then max_split = str\len()

    start = 1
    prev = 1
    do_split = () ->
      if not str or str\len() == 0 then return

      local first, last
      if max_split > 0
        first, last = regex.search @_regex, str, start

      if not first or first > str\len()
        ret = str\sub prev, str\len()
        str = nil
        return ret

      ret = str\sub prev, first - 1
      prev = last + 1

      start = 1 + if start >= last then start else last

      if skip_empty and ret\len() == 0
        do_split()
      else
        max_split -= 1
        ret

    do_split

  split: (str, skip_empty, max_split) =>
    @_check_self!
    check_arg str, 'string', 2, 'split', @_level
    [v for v in @gsplit str, skip_empty, max_split]

  gfind: (str) =>
    @_check_self!
    check_arg str, 'string', 2, 'gfind', @_level

    start = 1
    ->
      first, last = regex.search(@_regex, str, start)
      return unless first

      start = if last >= start then last + 1 else start + 1
      str\sub(first, last), first, last

  find: (str) =>
    @_check_self!
    check_arg str, 'string', 2, 'find', @_level

    ret = [str: s, first: f, last: l for s, f, l in @gfind(str)]
    next(ret) and ret

  sub: (str, repl, max_count) =>
    @_check_self!
    check_arg str, 'string', 2, 'sub', @_level
    if max_count != nil
      check_arg max_count, 'number', 4, 'sub', @_level

    max_count = str\len() + 1 if not max_count or max_count == 0

    if type(repl) == 'function'
       do_replace_fun @, repl, str, max_count
    elseif type(repl) == 'string'
      regex.replace @_regex, repl, str, max_count
    else
      error "Argument 2 to sub should be a string or function, is '#{type(repl)}' (#{repl})", @_level

  gmatch: (str, start) =>
    @_check_self!
    check_arg str, 'string', 2, 'gmatch', @_level
    start = if start then start - 1 else 0

    match = regex.match @_regex, str, start
    i = 1
    ->
      return unless match
      first, last = regex.get_match match, i
      return unless first
      i += 1

      {
        str: str\sub first + start, last + start
        first: first + start
        last: last + start
      }

  match: (str, start) =>
    @_check_self!
    check_arg(str, 'string', 2, 'match', @_level)

    ret = [v for v in @gmatch str, start]
    -- Return nil rather than a empty table so that if re.match(...) works
    return nil if next(ret) == nil
    ret

-- Create a regex object from a pattern, flags, and error depth
real_compile = (pattern, level, flags, stored_level) ->
  if pattern == ''
    error 'Regular expression must not be empty', level + 1

  re = regex.compile pattern, flags
  if type(re) == 'string'
    error regex, level + 1

  RegEx re, stored_level or level + 1

-- Compile a pattern then invoke a method on it
invoke = (str, pattern, fn, flags, ...) ->
  compiled_regex = real_compile(pattern, 3, flags)
  compiled_regex[fn](compiled_regex, str, ...)

-- Generate a static version of a method with arg type checking
gen_wrapper = (impl_name) ->
  (str, pattern, ...) ->
    check_arg str, 'string', 1, impl_name, 2
    check_arg pattern, 'string', 2, impl_name, 2
    invoke str, pattern, impl_name, unpack_args ...

-- And now at last the actual public API
re = regex.init_flags(re)

re.compile = (pattern, ...) ->
  check_arg pattern, 'string', 1, 'compile', 2
  real_compile pattern, 2, regex.process_flags(...), 2

re.split  = gen_wrapper 'split'
re.gsplit = gen_wrapper 'gsplit'
re.find   = gen_wrapper 'find'
re.gfind  = gen_wrapper 'gfind'
re.match  = gen_wrapper 'match'
re.gmatch = gen_wrapper 'gmatch'
re.sub    = gen_wrapper 'sub'

re

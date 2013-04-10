local regex = aegisub.__init_regex()
local select_first
select_first = function(n, a, ...)
  if n == 0 then
    return 
  end
  return a, select_first(n - 1, ...)
end
local unpack_args
unpack_args = function(...)
  local userdata_start = nil
  for i = 1, select('#', ...) do
    local v = select(i, ...)
    if type(v) == 'userdata' then
      userdata_start = i
      break
    end
  end
  if not (userdata_start) then
    return 0, ...
  end
  local flags = regex.process_flags(select(userdata_start, ...))
  if type(flags) == 'string' then
    error(flags, 3)
  end
  return flags, select_first(userdata_start - 1, ...)
end
local check_arg
check_arg = function(arg, expected_type, argn, func_name, level)
  if type(arg) ~= expected_type then
    return error("Argument " .. tostring(argn) .. " to " .. tostring(func_name) .. " should be a '" .. tostring(expected_type) .. "', is '" .. tostring(type(arg)) .. "' (" .. tostring(arg) .. ")", level + 1)
  end
end
local replace_match
replace_match = function(match, func, str, last, acc)
  if last < match.last then
    acc[#acc + 1] = str:sub(last, match.first - 1)
  end
  local repl = func(match.str, match.first, match.last)
  if type(repl) == 'string' then
    acc[#acc + 1] = repl
  else
    acc[#acc + 1] = match.str
  end
  return match.first, match.last + 1
end
local do_single_replace_fun
do_single_replace_fun = function(re, func, str, acc, pos)
  local matches = re:match(str, pos)
  if not (matches) then
    return pos
  end
  local start
  if #matches == 1 then
    start = 1
  else
    start = 2
  end
  local last = pos
  local first
  for i = start, #matches do
    first, last = replace_match(matches[i], func, str, last, acc)
  end
  if first == last then
    acc[#acc + 1] = str:sub(last, last)
    last = last + 1
  end
  return last, matches[1].first <= str:len()
end
local do_replace_fun
do_replace_fun = function(re, func, str, max)
  local acc = { }
  local pos = 1
  local i
  for i = 1, max do
    local more
    pos, more = do_single_replace_fun(re, func, str, acc, pos)
    if not (more) then
      max = i
      break
    end
  end
  return table.concat(acc, '') .. str:sub(pos)
end
local RegEx
do
  local start
  local _parent_0 = nil
  local _base_0 = {
    _check_self = function(self)
      if not (self.__class == RegEx) then
        return error('re method called with invalid self. You probably used . when : is needed.', 3)
      end
    end,
    gsplit = function(self, str, skip_empty, max_split)
      self:_check_self()
      check_arg(str, 'string', 2, 'gsplit', self._level)
      if not max_split or max_split <= 0 then
        max_split = str:len()
      end
      start = 1
      local prev = 1
      local do_split
      do_split = function()
        if not str or str:len() == 0 then
          return 
        end
        local first, last
        if max_split > 0 then
          first, last = regex.search(self._regex, str, start)
        end
        if not first or first > str:len() then
          local ret = str:sub(prev, str:len())
          str = nil
          return ret
        end
        local ret = str:sub(prev, first - 1)
        prev = last + 1
        start = 1 + (function()
          if start >= last then
            return start
          else
            return last
          end
        end)()
        if skip_empty and ret:len() == 0 then
          return do_split()
        else
          max_split = max_split - 1
          return ret
        end
      end
      return do_split
    end,
    split = function(self, str, skip_empty, max_split)
      self:_check_self()
      check_arg(str, 'string', 2, 'split', self._level)
      return (function()
        local _accum_0 = { }
        local _len_0 = 1
        for v in self:gsplit(str, skip_empty, max_split) do
          _accum_0[_len_0] = v
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)()
    end,
    gfind = function(self, str)
      self:_check_self()
      check_arg(str, 'string', 2, 'gfind', self._level)
      start = 1
      return function()
        local first, last = regex.search(self._regex, str, start)
        if not (first) then
          return 
        end
        if last >= start then
          start = last + 1
        else
          start = start + 1
        end
        return str:sub(first, last), first, last
      end
    end,
    find = function(self, str)
      self:_check_self()
      check_arg(str, 'string', 2, 'find', self._level)
      local ret = (function()
        local _accum_0 = { }
        local _len_0 = 1
        for s, f, l in self:gfind(str) do
          _accum_0[_len_0] = {
            str = s,
            first = f,
            last = l
          }
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)()
      return next(ret) and ret
    end,
    sub = function(self, str, repl, max_count)
      self:_check_self()
      check_arg(str, 'string', 2, 'sub', self._level)
      if max_count ~= nil then
        check_arg(max_count, 'number', 4, 'sub', self._level)
      end
      if not max_count or max_count == 0 then
        max_count = str:len() + 1
      end
      if type(repl) == 'function' then
        return do_replace_fun(self, repl, str, max_count)
      elseif type(repl) == 'string' then
        return regex.replace(self._regex, repl, str, max_count)
      else
        return error("Argument 2 to sub should be a string or function, is '" .. tostring(type(repl)) .. "' (" .. tostring(repl) .. ")", self._level)
      end
    end,
    gmatch = function(self, str, start)
      self:_check_self()
      check_arg(str, 'string', 2, 'gmatch', self._level)
      if start then
        start = start - 1
      else
        start = 0
      end
      local match = regex.match(self._regex, str, start)
      local i = 1
      return function()
        if not (match) then
          return 
        end
        local first, last = regex.get_match(match, i)
        if not (first) then
          return 
        end
        i = i + 1
        return {
          str = str:sub(first + start, last + start),
          first = first + start,
          last = last + start
        }
      end
    end,
    match = function(self, str, start)
      self:_check_self()
      check_arg(str, 'string', 2, 'match', self._level)
      local ret = (function()
        local _accum_0 = { }
        local _len_0 = 1
        for v in self:gmatch(str, start) do
          _accum_0[_len_0] = v
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)()
      if next(ret) == nil then
        return nil
      end
      return ret
    end
  }
  _base_0.__index = _base_0
  if _parent_0 then
    setmetatable(_base_0, _parent_0.__base)
  end
  local _class_0 = setmetatable({
    __init = function(self, _regex, _level)
      self._regex, self._level = _regex, _level
    end,
    __base = _base_0,
    __name = "RegEx",
    __parent = _parent_0
  }, {
    __index = function(cls, name)
      local val = rawget(_base_0, name)
      if val == nil and _parent_0 then
        return _parent_0[name]
      else
        return val
      end
    end,
    __call = function(cls, ...)
      local _self_0 = setmetatable({}, _base_0)
      cls.__init(_self_0, ...)
      return _self_0
    end
  })
  _base_0.__class = _class_0
  local self = _class_0
  start = 1
  if _parent_0 and _parent_0.__inherited then
    _parent_0.__inherited(_parent_0, _class_0)
  end
  RegEx = _class_0
end
local real_compile
real_compile = function(pattern, level, flags, stored_level)
  if pattern == '' then
    error('Regular expression must not be empty', level + 1)
  end
  local re = regex.compile(pattern, flags)
  if type(re) == 'string' then
    error(regex, level + 1)
  end
  return RegEx(re, stored_level or level + 1)
end
local invoke
invoke = function(str, pattern, fn, flags, ...)
  local compiled_regex = real_compile(pattern, 3, flags)
  return compiled_regex[fn](compiled_regex, str, ...)
end
local gen_wrapper
gen_wrapper = function(impl_name)
  return function(str, pattern, ...)
    check_arg(str, 'string', 1, impl_name, 2)
    check_arg(pattern, 'string', 2, impl_name, 2)
    return invoke(str, pattern, impl_name, unpack_args(...))
  end
end
local re = regex.init_flags(re)
re.compile = function(pattern, ...)
  check_arg(pattern, 'string', 1, 'compile', 2)
  return real_compile(pattern, 2, regex.process_flags(...), 2)
end
re.split = gen_wrapper('split')
re.gsplit = gen_wrapper('gsplit')
re.find = gen_wrapper('find')
re.gfind = gen_wrapper('gfind')
re.match = gen_wrapper('match')
re.gmatch = gen_wrapper('gmatch')
re.sub = gen_wrapper('sub')
return re

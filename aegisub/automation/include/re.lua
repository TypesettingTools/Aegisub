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
--
-- $Id$

-- Get the wxRegex binding
local regex = aegisub.__init_regex()

-- Compiled regular expression type protoype
local re_proto = {}
local re_proto_mt = { __index = re_proto }

-- Convert an iterator to an array
local function to_table(...)
    local arr = {}
    local i = 1
    for v in ... do
        arr[i] = v
        i = i + 1
    end
    return arr
end

-- Return the first n elements from ...
local function select_first(n, a, ...)
    if n == 0 then return end
    return a, select_first(n - 1, ...)
end

-- Extract the flags from ..., bitwise OR them together, and move them to the
-- front of ...
local function unpack_args(...)
    local n = select('#', ...)
    local userdata_start = nil
    for i = 1, n do
        local v = select(i, ...)
        if type(v) == "userdata" then
            userdata_start = i
            break
        end
    end

    if not userdata_start then
        return 0, ...
    end

    flags = regex.process_flags(select(userdata_start, ...))
    if type(flags) == "string" then
        error(flags, 3)
    end

    return flags, select_first(userdata_start - 1, ...)
end

-- Verify that a valid value was passed for self
local function check_self(self)
    if getmetatable(self) ~= re_proto_mt then
        error("re method called with invalid self. You probably used . when : is needed.", 3)
    end
end

-- Typecheck a variable and throw an error if it fails
local function check_arg(arg, expected_type, argn, func_name, level)
    if type(arg) ~= expected_type then
        error(
            string.format("Argument %d to %s should be a '%s', is '%s' (%s)",
                argn, func_name, expected_type, type(arg), tostring(arg)),
            level + 1)
    end
end

function re_proto.gsplit(self, str, skip_empty, max_split)
    check_self(self)
    check_arg(str, "string", 2, "gsplit", self._level)
    if not max_split or max_split <= 0 then max_split = str:len() end

    local function do_split()
        if not str or str:len() == 0 then
            return nil
        end

        if max_split == 0 or not regex.matches(self._regex, str) then
            local ret = str
            str = nil
            return ret
        end

        local first, last = regex.get_match(self._regex, str, 0)
        local ret = str:sub(1, first - 1)
        str = str:sub(last + 1)

        if skip_empty and ret:len() == 0 then
            return do_split()
        else
            max_split = max_split - 1
            return ret
        end
    end

    return do_split
end

function re_proto.split(self, str, skip_empty, max_split)
    check_self(self)
    check_arg(str, "string", 2, "split", self._level)
    return to_table(self:gsplit(str, skip_empty, max_split))
end

function re_proto.gfind(self, str)
    check_self(self)
    check_arg(str, "string", 2, "gfind", self._level)

    local offset = 0
    return function()
        local has_matches = regex.matches(self._regex, str)
        if not has_matches then return end

        local first, last = regex.get_match(self._regex, str, 0)
        local ret = str:sub(first, last)
        str = str:sub(last + 1)

        last = last + offset
        offset = offset + first
        return ret, offset, last
    end
end

function re_proto.find(self, str)
    check_self(self)
    check_arg(str, "string", 2, "find", self._level)

    local i = 1
    local ret = {}
    for s, f, l in self:gfind(str) do
        ret[i] = {
            str   = s,
            first = f,
            last  = l
        }
        i = i + 1
    end
    return ret
end

-- Replace a match with the value returned from func when passed the match
local function replace_match(match, func, str, last, acc)
    if last < match.last then
        acc[#acc + 1] = str:sub(last, match.first - 1)
    end

    local ret = func(match.str, match.first, match.last)
    if type(ret) == "string" then
        acc[#acc + 1] = ret
    else
        -- If it didn't return a string just leave the old value
        acc[#acc + 1] = match.str
    end

    return match.last + 1
end

-- Replace all matches from a single iteration of the regexp
local function do_single_replace_fun(re, func, str, acc)
    local matches = re:match(str)

    -- No more matches so just return what we have so far
    if not matches then
        return str
    end

    -- One match means no capturing groups, so pass the entire thing to
    -- the replace function
    if #matches == 1 then
        local rest = replace_match(matches[1], func, str, 1, acc)
        return str:sub(rest), true
    end

    -- Multiple matches means there were capture groups, so skip the first one
    -- and pass the rest to the replace function
    local last = 1
    for i = 2, #matches do
        last = replace_match(matches[i], func, str, last, acc)
    end

    return str:sub(last), true
end

local function do_replace_fun(re, func, str, max)
    local acc = {}
    local i
    for i = 1, max do
        str, continue = do_single_replace_fun(re, func, str, acc)
        if not continue then max = i end
    end
    return table.concat(acc, "") .. str, max
end

function re_proto.sub(self, str, repl, count)
    check_self(self)
    check_arg(str, "string", 2, "sub", self._level)
    if count ~= nil then
        check_arg(count, "number", 4, "sub", self._level)
    end

    if not count or count == 0 then count = str:len() end

    if type(repl) == "function" then
       return do_replace_fun(self, repl, str, count)
    elseif type(repl) == "string" then
        return regex.replace(self._regex, repl, str, count)
    else
        error(
            string.format("Argument 2 to sub should be a string or function, is '%s' (%s)",
                type(repl), tostring(repl)),
            self._level)
    end
end

function re_proto.gmatch(self, str)
    check_self(self)
    check_arg(str, "string", 2, "gmatch", self._level)

    local match_count = regex.match_count(self._regex, str)
    local i = 0
    return function()
        if i == match_count then return end
        i = i + 1
        local first, last = regex.get_match(self._regex, str, i - 1)
        return {
            str   = str:sub(first, last),
            first = first,
            last  = last
        }
    end
end

function re_proto.match(self, str)
    check_self(self)
    check_arg(str, "string", 2, "match", self._level)

    local ret = to_table(self:gmatch(str))
    -- Return nil rather than a empty table so that if re.match(...) works
    if next(ret) == nil then return end
    return ret
end

-- Create a wxRegExp object from a pattern, flags, and error depth
local function real_compile(pattern, level, flags, stored_level)
    local regex = regex.compile(pattern, flags)
    if not regex then
        error("Bad syntax in regular expression", level + 1)
    end
    return setmetatable({
            _regex = regex,
            _level = stored_level or level + 1
        },
        re_proto_mt)
end

-- Compile a pattern then invoke a method on it
local function invoke(str, pattern, fn, flags, ...)
    local comp = real_compile(pattern, 3, flags)
    return comp[fn](comp, str, ...)
end

-- Generate a static version of a method with arg type checking
local function gen_wrapper(impl_name)
    return function(str, pattern, ...)
        check_arg(str, "string", 1, impl_name, 2)
        check_arg(pattern, "string", 2, impl_name, 2)
        return invoke(str, pattern, impl_name, unpack_args(...))
    end
end

-- And now at last the actual public API
local re = regex.init_flags(re)

function re.compile(pattern, ...)
    check_arg(pattern, "string", 1, "compile", 2)
    return real_compile(pattern, 2, regex.process_flags(...), 2)
end

re.split  = gen_wrapper("split")
re.gsplit = gen_wrapper("gsplit")
re.find   = gen_wrapper("find")
re.gfind  = gen_wrapper("gfind")
re.match  = gen_wrapper("match")
re.gmatch = gen_wrapper("gmatch")
re.sub    = gen_wrapper("sub")

_G.re = re
return _G.re

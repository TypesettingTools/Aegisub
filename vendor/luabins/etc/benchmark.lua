-- This benchmark is compatible with luamarca benchmarking system
-- http://github.com/agladysh/luamarca

package.cpath = "./?.so;"..package.cpath

local luabins = require("luabins")

local table_concat = table.concat
local loadstring, assert = loadstring, assert
local pairs, type, tostring = pairs, type, tostring
local luabins_save, luabins_load = luabins.save, luabins.load

local lua = ([[return {
  true, false, 42, "string",
  [{
    true, false, 42, "string",
    [true] = true, [false] = false, [42] = 42, string = "string"
  }] =
  {
    true, false, 42, "string",
    [true] = true, [false] = false, [42] = 42, string = "string"
  }
}]]):gsub("[%s\n]+", "") -- Remove spaces for compactness

local data = assert(loadstring(lua))()
local saved = assert(luabins_save(data))

-- Imagine we know exact data structure.
-- We still impose some overhead on table.concat() related
-- stuff, since it is more realistic scenario.
-- Still looks a bit silly.
local concat = function(data)
  local buf = {}
  local function cat(v) buf[#buf + 1] = tostring(v) return cat end

  -- Find table key
  local tablekey, tableval
  for k, v in pairs(data) do
    if type(k) == "table" then
      tablekey, tableval = k, v
      break
    end
  end

  cat 'return{'

  cat (data[1]) ','
  cat (data[2]) ','
  cat (data[3]) ','
  cat '"' (data[4]) '",'

    cat '[{'

    cat (tablekey[1]) ','
    cat (tablekey[2]) ','
    cat (tablekey[3]) ','
    cat '"' (tablekey[4]) '",'

    cat '[' (true) ']=' (tablekey[true]) ','
    cat '[' (false) ']=' (tablekey[false]) ','
    cat '[' (42) ']=' (tablekey[42]) ','
    cat 'string' '=' '"' (tablekey["string"]) '"'

    cat '}]='

    cat '{'

    cat (tablekey[1]) ','
    cat (tablekey[2]) ','
    cat (tablekey[3]) ','
    cat '"' (tablekey[4]) '",'

    cat '[' (true) ']=' (tablekey[true]) ','
    cat '[' (false) ']=' (tablekey[false]) ','
    cat '[' (42) ']=' (tablekey[42]) ','
    cat 'string' '=' '"' (tablekey["string"]) '"'

    cat '}'

  cat '}'

  return table_concat(buf, '')
end

-- Sanity check
assert(concat(data) == lua)

local bench = {}

bench.concat = function()
  assert(concat(data))
end

bench.loadstring = function()
  assert(loadstring(lua))()
end

bench.luabins_save = function()
  assert(luabins_save(data))
end

bench.luabins_load = function()
  assert(luabins_load(saved))
end

return bench

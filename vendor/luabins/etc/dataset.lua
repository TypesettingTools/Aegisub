-- Random Luabins dataset generator

package.cpath = "./lib/?.so;"..package.cpath

local luabins = require("luabins")

local luabins_save, luabins_load = luabins.save, luabins.load

math.randomseed(123456)

-- TODO: Generalize. Copy-paste from test.lua
local function gen_random_dataset(num, nesting)
  num = num or math.random(0, 128)
  nesting = nesting or 1

  local gen_str = function()
    local t = {}
    local n = math.random(0, 1024)
    for i = 1, n do
      t[i] = string.char(math.random(0, 255))
    end
    return table.concat(t)
  end

  local gen_bool = function() return math.random() >= 0.5 end

  local gen_nil = function() return nil end

  local generators =
  {
    gen_nil;
    gen_nil;
    gen_nil;
    gen_bool;
    gen_bool;
    gen_bool;
    function() return math.random() end;
    function() return math.random(-10000, 10000) end;
    function() return math.random() * math.random(-10000, 10000) end;
    gen_str;
    gen_str;
    gen_str;
    function()
      if nesting >= 24 then
        return nil
      end

      local t = {}
      local n = math.random(0, 24 - nesting)
      for i = 1, n do
        local k = gen_random_dataset(1, nesting + 1)
        if k == nil then
          k = "(nil)"
        end
        t[ k ] = gen_random_dataset(
            1,
            nesting + 1
          )
      end

      return t
    end;
  }

  local t = {}
  for i = 1, num do
    local n = math.random(1, #generators)
    t[i] = generators[n]()
  end
  return unpack(t, 0, num)
end

local saved = assert(luabins_save(gen_random_dataset()))

local filename = select(1, ...)
assert(filename, "Usage: lua dataset.lua <out_filename>")

local file
if filename == "-" then
  file = io.stdout
else
  file = assert(io.open(filename, "w"))
end

file:write(saved)

if file ~= io.stdout then
  file:close()
end
file = nil

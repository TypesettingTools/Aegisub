-- Luabins to Lua converter
-- Requires lua-nucleo (http://github.com/lua-nucleo/lua-nucleo/)

package.cpath = "./lib/?.so;"..package.cpath

local luabins = require 'luabins'

dofile('lua-nucleo/strict.lua')
dofile('lua-nucleo/import.lua')

local tserialize = import 'lua-nucleo/tserialize.lua' { 'tserialize' }

local filename = select(1, ...)
assert(filename, "Usage: lua tolua.lua <out_filename>")

local file
if filename == "-" then
  file = io.stdin
else
  file = assert(io.open(filename, "r"))
end

io.write(
    tserialize(
        assert(
            luabins.load(
                file:read("*a")
              )
          )
      )
  )

io:flush()

if file ~= io.stdin then
  file:close()
end
file = nil


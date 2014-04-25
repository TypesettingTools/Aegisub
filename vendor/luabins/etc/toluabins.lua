-- Lua to Luabins converter

package.cpath = "./lib/?.so;"..package.cpath

local luabins = require 'luabins'

local filename = select(1, ...)
assert(filename, "Usage: lua toluabins.lua <out_filename>")

io.write(
    luabins.save(
        assert(loadfile(filename))()
      )
  )

io:flush()

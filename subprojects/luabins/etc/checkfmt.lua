--- Luabins format checker

package.cpath = "./lib/?.so;"..package.cpath

local luabins = require("luabins")

local luabins_save, luabins_load = luabins.save, luabins.load

local filename = select(1, ...)
assert(filename, "Usage: lua checkfmt.lua <out_filename>")

local file
if filename == "-" then
  file = io.stdin
else
  file = assert(io.open(filename, "r"))
end

assert(
    luabins_load(
        file:read("*a")
      )
  )

if file ~= io.stdin then
  file:close()
end
file = nil

print("OK")

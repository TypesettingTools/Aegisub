local ffi = require("ffi")

ffi.cdef[[
  typedef struct timeval {
    long tv_sec;
    long tv_usec;
  } timeval;

  int gettimeofday(timeval *t, void *tzp);
]]

-- busted depends on luasocket just for gettime(), so just supply a definition
-- of that to avoid the dep
package.loaded['socket'] = {
  gettime = function()
    local t = ffi.new("timeval")
    ffi.C.gettimeofday(t, nil)
    return tonumber(t.tv_sec) + tonumber(t.tv_usec) / 1000000.0
  end
}

require 'busted.runner'({ batch = true, standalone = false })

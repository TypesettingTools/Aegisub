local ffi = require("ffi")

ffi.cdef[[
  typedef struct timeval {
    long tv_sec;
    long tv_usec;
  } timeval;

  int gettimeofday(timeval *t, void *tzp);

  typedef struct timespec {
    long tv_sec;
    long tv_nsec;
  } timespec;
  int clock_gettime(int clk_id, timespec *tp);
]]

function gettime()
  local t = ffi.new("timeval")
  ffi.C.gettimeofday(t, nil)
  return tonumber(t.tv_sec) + tonumber(t.tv_usec) / 1000000.0
end
function monotime()
  local ts = ffi.new("timespec")
  ffi.C.clock_gettime(6, ts)
  return tonumber(ts.tv_sec) + tonumber(ts.tv_nsec) / 1000000000.0
end
function sleep()
end

-- busted depends on luasocket just for gettime(), so just supply a definition
-- of that to avoid the dep
package.loaded['socket'] = {
  gettime = gettime
}
package.loaded['system'] = {
  gettime = gettime,
  monotime = monotime,
  sleep = sleep
}

package.loaded['term'] = {
  isatty = function() return true end
}

require 'busted.runner'({ batch = true, standalone = false })

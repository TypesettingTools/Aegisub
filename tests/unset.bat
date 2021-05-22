cd %1

icacls data\file_access_denied /grant %USERNAME%:F
icacls data\file_read_only /grant %USERNAME%:W
icacls data\dir_access_denied /grant %USERNAME%:F
icacls data\dir_read_only /grant %USERNAME%:W

rmdir /s/q data

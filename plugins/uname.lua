--[[
    POSIX uname system information
]]

local function uname_info(param)
    return io.popen("uname -"..param):read()
end

sysname  = uname_info('s')
nodename = uname_info('n')
release  = uname_info('r')
version  = uname_info('v')
machine  = uname_info('m')

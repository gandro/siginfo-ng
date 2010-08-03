--[[
    POSIX uname system information
]]

using "OS"

local function uname(param)
    -- remove newline and whitespaces at the end
    return string.match(siginfo.ng.readexec("uname -"..param), "(.-)%s*$")
end

if __init__ then
    OS.SYSNAME  = uname("s")
    OS.NODENAME = uname("n")
    OS.RELEASE  = uname("r")
    OS.VERSION  = uname("v")
    OS.MACHINE  = uname("m")
end

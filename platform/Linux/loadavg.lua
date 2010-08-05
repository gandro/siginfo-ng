--[[
    linux load average
]]

using "LOAD"

local avg01, avg05, avg15, proc =
    string.match(siginfo.ng.readfile("/proc/loadavg"),
    "([%d%.]+) ([%d%.]+) ([%d%.]+) %d+/(%d+)"
)

LOAD.AVG_01 = avg01
LOAD.AVG_05 = avg05
LOAD.AVG_15 = avg15
LOAD.PROCESSES = proc

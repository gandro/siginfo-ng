--[[
    linux uptime
]]

using "UPTIME"

UPTIME.TOTAL.SECONDS = string.match(siginfo.ng.readfile("/proc/uptime"), "%d+")
UPTIME.TOTAL.MINUTES = math.floor(UPTIME.TOTAL.SECONDS / 60)
UPTIME.TOTAL.HOURS   = math.floor(UPTIME.TOTAL.MINUTES / 60)
UPTIME.TOTAL.DAYS    = math.floor(UPTIME.TOTAL.HOURS / 24)
UPTIME.TOTAL.YEARS   = math.floor(UPTIME.TOTAL.DAYS / 365)

UPTIME.SECONDS = math.floor(UPTIME.TOTAL.SECONDS % 60)
UPTIME.MINUTES = math.floor(UPTIME.TOTAL.MINUTES % 60)
UPTIME.HOURS   = math.floor(UPTIME.TOTAL.HOURS % 24)
UPTIME.DAYS    = math.floor(UPTIME.TOTAL.DAYS  % 365)
UPTIME.YEARS   = UPTIME.TOTAL.YEARS

UPTIME.HHMMSS  = string.format("%02i:%02i:%02i",
                     UPTIME.TOTAL.HOURS, UPTIME.MINUTES, UPTIME.SECONDS)
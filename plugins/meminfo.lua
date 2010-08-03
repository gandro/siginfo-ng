--[[
    memory information on linux
]]

using "RAM"
using "SWAP"

-- read /proc/meminfo to table
meminfo = {}
for line in io.lines("/proc/meminfo") do
    local key, value = string.match(line, "(.+):%s*(%d+)")
    meminfo[key] = math.floor(value / 1000)
end


RAM.CACHED = meminfo["Cached"] + meminfo["Buffers"]

RAM.TOTAL  = meminfo["MemTotal"]
RAM.FREE   = meminfo["MemFree"]
RAM.USED   = RAM.TOTAL - RAM.FREE - RAM.CACHED

SWAP.TOTAL = meminfo["SwapTotal"]
SWAP.FREE  = meminfo["SwapFree"]
SWAP.USED  = SWAP.TOTAL - SWAP.FREE

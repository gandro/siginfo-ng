--[[
    memory information on linux
]]

-- read /proc/meminfo to table
local meminfo = {}
for line in io.lines("/proc/meminfo") do
    local key, value = string.match(line, "(.+):%s*(%d+)")
    meminfo[key] = math.floor(value / 1000)
end

-- cache = meminfo["Cached"] + meminfo["Buffers"]

ram = {
    total = meminfo["MemTotal"],
--    free = meminfo["MemFree"] + cache,
--    used = meminfo["MemTotal"] - meminfo["MemFree"] - cache
}

swap  = {
    total = meminfo["SwapTotal"],
--    free  = meminfo["SwapFree"],
--    used  = meminfo["SwapTotal"] - meminfo["SwapFree"]
}

--[[
    CPU information for x86 processors on linux
]]

for line in io.lines("/proc/cpuinfo") do
    model = string.match(line, "model name%s*: (.+)") or model
    mhz   = string.match(line, "cpu MHz%s*: ([%d]+)") or mhz
    cache = string.match(line, "cache size%s*: (.+)") or cache
    cores = string.match(line, "cpu cores%s*: (%d+)") or cores
end

-- remove unnecessary whitespaces
if model then
    model = string.gsub(model, "(%s+)", " ")
end

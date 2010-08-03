--[[
    CPU information on linux
]]

using "CPU"

if __init__ then
    -- read /proc/cpuinfo to table
    cpuinfo = {}
    for line in io.lines("/proc/cpuinfo") do
        local key, value = string.match(line, "(.-)%s*: (.+)")
        if key and value then
            cpuinfo[key] = value
        end
    end

    CPU.MODEL = string.gsub(cpuinfo["model name"], "(%s+)", " ")
    CPU.MHZ   = math.floor(cpuinfo["cpu MHz"])
    CPU.CACHE = cpuinfo["cache size"]
    CPU.CORES = cpuinfo["cpu cores"] or 1
end

--[[
    linux cpu information
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

    if siginfo.ng.arch == "i386" or siginfo.ng.arch == "x86_64" then

        CPU.MODEL = string.gsub(cpuinfo["model name"], "(%s+)", " ")
        CPU.MHZ   = math.floor(cpuinfo["cpu MHz"])
        CPU.CACHE = cpuinfo["cache size"]
        CPU.CORES = cpuinfo["cpu cores"] or 1

    elseif siginfo.ng.arch == "sparc" or siginfo.ng.arch == "sparc64" then

        CPU.MHZ = math.floor(tonumber(cpuinfo["Cpu0ClkTck"], 16) / 1000000)

        -- August 1, 2010 by Wynton "crazycusti" Tietjen
        -- dedicated to Nadine
        CPU.MODEL = string.gsub(cpuinfo["cpu"], "(%s+)", " ")
        CPU.CORES = cpuinfo["ncpus active"] or 1

    end
end

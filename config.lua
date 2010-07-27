--[[
    siginfo-ng configuration file
]]

-- username
siginfo.username = "noname"
-- password (as sha1 hash)
siginfo.password = "da39a3ee5e6b4b0d3255bfef95601890afd80709"

-- computer profile
siginfo.computer = "default-pc"

-- load plugins
load_plugin_dir(__path.."/plugins")

siginfo.uptime = uptime.seconds

siginfo.layout = {
    row1 = { "Client: ", siginfo.version, " - now with Lua scripting support :)" },
    row2 = { "Processor: ", cpu.model },
    row3 = { "CPU-Clock: ", cpu.mhz, " MHz, CPU-Cache: ", cpu.cache, ", CPU-Cores: ", cpu.cores,
                ", RAM: ", mem.ram.total, "MB, SWAP: ", mem.swap.total, "MB" },
    row4 = { "OS: ", uname.sysname, " ", uname.release, ", Architecture: ", uname.machine,
                ", Hostname: ", uname.nodename },
    row5 = { "Check out my profile on http://", siginfo.server,
                "/vcards/showUser/", siginfo.username, ".html" }
}

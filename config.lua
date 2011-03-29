--[[
    siginfo-ng configuration file
]]

-- username
siginfo.username = "noname"
-- password (as sha1 hash)
siginfo.password = "da39a3ee5e6b4b0d3255bfef95601890afd80709"

-- computer profile
siginfo.computer = "default-pc"

siginfo.layout = {
    row1 = {
        "CPU: ", CPU.MODEL, " (", CPU.MHZ, " MHz, ", CPU.CACHE, " Cache, ", CPU.CORES, " Cores)"
    },
    row2 = {
        "RAM: ", RAM.USED, " of ", RAM.TOTAL, " MB used, ", RAM.FREE, "MB free - ",
        "Swap: ", SWAP.USED, " of ", SWAP.TOTAL, " MB used, ", SWAP.FREE, "MB free"
    },
    row3 = {
        "OS: ", OS.SYSNAME, " ", OS.RELEASE, " machine ", OS.MACHINE, " on ", OS.NODENAME,
        ", ", LOAD.PROCESSES, " processes running"
    },
    row4 = {
        "HDD: /home: ", HDD.USED("/home"), " MiB of ", HDD.TOTAL("/home"),
        " MiB used, ", HDD.FREE("/home"), " MiB free"
    },
    row5 = {
        "Uptime: ", UPTIME.TOTAL.DAYS, " days, ", UPTIME.HOURS, " hours, ",
        UPTIME.MINUTES, " minutes and ", UPTIME.SECONDS, " seconds, ",
        LOAD.AVG_01, " ", LOAD.AVG_05, " ", LOAD.AVG_15, " load average"
    }
}

siginfo.uptime = UPTIME.TOTAL.SECONDS

--[[
    siginfo-ng configuration file
]]

-- username
siginfo.username = "gandro"
-- password (as sha1 hash)
siginfo.password = "c95397644dd9b9d4a2eaaf25575132c35243d615"

-- computer profile
siginfo.computer = "default-pc"

siginfo.interval = 5

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

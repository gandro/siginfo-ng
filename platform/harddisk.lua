--[[
    posix harddisk information
]]

using "HDD"

-- default blocksize is MiB
blocksize = 2^20

local function df(partition)
    local total, used, free, percentage = string.match(
        siginfo.ng.readexec("df -k -P "..partition),
        "(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%%"
    )
    local bs = blocksize / 1024

    return {
        total = math.floor(total / bs),
        used  = math.floor(used / bs),
        free  = math.floor(free / bs),
        percentage = percentage
    }
end

HDD.TOTAL = function(partition)
    return df(partition).total
end

HDD.FREE = function(partition)
    return df(partition).free
end

HDD.USED = function(partition)
    return df(partition).used
end

HDD.PERCENTAGE = function(partition)
    return df(partition).percentage
end

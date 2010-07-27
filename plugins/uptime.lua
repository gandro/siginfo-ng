--[[
    linux uptime
]]

function seconds()
    return string.match(io.open("/proc/uptime"):read(), "%d+")
end

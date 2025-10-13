playerId = 0
local c = 0

function Init()
    playerId = GetEntity("Player")
    DebugLog(playerId)

end

function Update()
    DebugLog("UpdateCountManager")
    c = GetEntityScriptGlobal(playerId,"Move.lua","maxCount")
    DebugLog(c) 
    DebugLog("EndCountManager")
end

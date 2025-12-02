local playerId =0

function Init()
    playerId = GetEntity("PlayerBar")
end

function Update()
    DebugLog(GetEntityScriptGlobal(playerId,"Echo.lua","echoCharge"))
    if GetEntityScriptGlobal(playerId,"Echo.lua","echoCharge") >= 100.0 then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.1)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.5)
    end
end

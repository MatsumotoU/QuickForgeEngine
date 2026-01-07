local playerId =0

function Init()
    playerId = GetEntity("PlayerBar")
end

function Update()
    if CountEntityTag("card") >= 1 then
        return
    end

    if GetEntityScriptGlobal(playerId,"Echo.lua","canEcho") then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.1)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.5)
    end
end

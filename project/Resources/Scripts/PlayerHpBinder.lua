local beatId = 0

function Init()
    beatId = GetEntity("Pacemaker")
    SetEntityScriptGlobal(this.GetEntityId(),"Numbers.lua","num",GetEntityScriptGlobal(beatId,"Pacemaker.lua","bpm"))
end

function Update()
    
end

function OnPlayerChangeBpm()
    DebugLog("Change")
    SetEntityScriptGlobal(this.GetEntityId(),"Numbers.lua","num",GetEntityScriptGlobal(beatId,"Pacemaker.lua","bpm"))
end

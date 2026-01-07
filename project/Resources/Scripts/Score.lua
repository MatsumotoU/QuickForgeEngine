function Init()
    DebugLog(GetScore())
    UpdateScore()
end

function Update()
    
end

function UpdateScore()
    SetEntityScriptGlobal(this.GetEntityId(),"Numbers.lua","num",GetScore())
end
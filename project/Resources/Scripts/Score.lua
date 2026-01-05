function Init()
    DebugLog(GetSceneGlobalData("Score"))
end

function Update()
    DebugLog(GetSceneGlobalData("Score"))
end

function UpdateScore()
    SetEntityScriptGlobal(this.GetEntityId(),"Numbers.lua","num",GetSceneGlobalData("Score"))
end
function Init()
    
end

function Update()

end

function UpdateScore()
    SetEntityScriptGlobal(this.GetEntityId(),"Numbers.lua","num",GetSceneGlobalData("Score"))
end
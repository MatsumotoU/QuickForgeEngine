objName = "name"
scriptName = "name"
varName = "name"

local map = {{},{}}

function Init()
    local id = GetEntity(objName)
    map = GetEntityScriptGlobal(id,scriptName,varName)
    DebugLog("ID: "..id)
    DebugLog("ScriptName: "..scriptName)
    DebugLog("VaarName: "..varName)

    DebugLog("Getter")
    for i = 1, #map, 1 do
       local row = map[i]
       for j = 1, #row, 1 do
            DebugLog(map[i][j])
       end
    end
end

function Update()
    if QFE.Input.GetKeyTrigger("Decide") then
        local id = GetEntity(objName)
        map = GetEntityScriptGlobal(id,scriptName,varName)
        DebugLog("Getter")
    for i = 1, #map, 1 do
       local row = map[i]
       for j = 1, #row, 1 do
            DebugLog(map[i][j])
       end
    end
        DebugLog("EndGetter")    
    end
end

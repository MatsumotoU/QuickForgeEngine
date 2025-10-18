objectName = "objectName"
scriptName = "ScriptName"
varName = "VarName"
local linkID = 0

nowNum = 0
numMax = 3

space = 64.0
local objTable = {}
uiObjName = "BulletNumUI.json"

function Init()
    linkID = GetEntity(objectName)
    DebugLog("LinkedID:"..linkID)
    numMax = GetEntityScriptGlobal(linkID,scriptName,varName)
    DebugLog("Obj:"..objectName .. " scriptName:" .. scriptName .. " varName:"..varName)
    nowNum = numMax
    tempTransform = Transform.new()
    tempTransform.translate.x = transform.translate.x
    tempTransform.translate.y = transform.translate.y

    for i = 1, numMax, 1 do
        table.insert(objTable,CreateEntity(uiObjName,tempTransform))
        DebugLog("AddObject :"..objTable[i])
        tempTransform.translate.x = tempTransform.translate.x + space
    end
end

function Update()
    -- 一応制限
    DebugLog("ID:"..linkID .. " scriptName:" .. scriptName .. " varName:"..varName)
    nowNum = GetEntityScriptGlobal(linkID,scriptName,varName)
    

    if nowNum > numMax then
        nowNum = numMax
    end

    -- 描画するかどうか
    for i = 1, numMax, 1 do
        local isDraw = GetIsDraw(objTable[i])
        if nowNum >= i then
           SetIsDraw(objTable[i],true)
        else
            SetIsDraw(objTable[i],false)
        end
    end
end

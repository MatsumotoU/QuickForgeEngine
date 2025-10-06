function Init()
DebugLog("Init")

local entityId = CreateEntity("anchor.obj.json")
DebugLog("Entity ID: " .. entityId)
DebugLog("X:" .. transform.translate.x)
SetPosition({transform.translate.x,transform.translate.y,transform.translate.z},entityId)

DebugLog("Generated!")
end

function Update()
end

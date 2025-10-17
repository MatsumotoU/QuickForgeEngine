targetObjName = "targetObjName"
offsetX = 1.0

local targetId = 0
local targetTransform = Transform.new()

function Init()
    targetId = GetEntity(targetObjName)
end

function Update()
    targetTransform = GetTransform(targetId)

    if transform.translate.x + offsetX <= targetTransform.translate.x then
        transform.translate.x = targetTransform.translate.x - offsetX
    end
end

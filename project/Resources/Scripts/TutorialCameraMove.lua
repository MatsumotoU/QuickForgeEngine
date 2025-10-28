targetObjName = "TutorialPlayer"
offsetX = 1.0

local targetId = 0
local targetTransform = Transform.new()

--[[
    初期化処理
--]]
function Init()
    targetId = GetEntity(targetObjName)
end

--[[
    更新処理
--]]
function Update()

    targetTransform = GetTransform(targetId)

    if transform.translate.x <= 42.0 then

    if transform.translate.x + offsetX <= targetTransform.translate.x then
        transform.translate.x = targetTransform.translate.x - offsetX
    end 
    end

end
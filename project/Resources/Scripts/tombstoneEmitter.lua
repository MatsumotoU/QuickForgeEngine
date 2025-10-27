particleObjName = "TombstoneParticle.json"

-- 生成する数
maxNum = 5

--[[
    初期化処理
--]]
function Init()
    local tempTransform = Transform.new()

    for i = 1, maxNum, 1 do    
        tempTransform.scale.x = 0.1 + math.random() * 0.4
        tempTransform.scale.y = 0.1 + math.random() * 0.4
        tempTransform.scale.z = 0.1 + math.random() * 0.4
        tempTransform.translate = transform.translate
        tempTransform.translate.y = transform.translate.y + 1.0
        tempTransform.rotate.x = math.random() * 6.28
        tempTransform.rotate.y = math.random() * 6.28
        tempTransform.rotate.z = math.random() * 6.28
        CreateEntity(particleObjName,tempTransform)
    end
end

--[[
    更新処理
--]]
function Update()
    destroy()
end
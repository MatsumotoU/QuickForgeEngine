smokeObjName = "SmokeParticle.json"
circleObjName = "CircleParticle.json"
hitEffectObjName = "hitEffectParticle.json"

-- 生成する数
local maxNum = 5

--[[
    初期化処理
--]]
function Init()
    local tempTransform = Transform.new()
    -- 円の演出を出現
    tempTransform.translate = transform.translate
    tempTransform.translate.y = transform.translate.y + 1.0
    tempTransform.translate.z = transform.translate.z - 1.0
    CreateEntity(circleObjName,tempTransform)

    -- ヒットエフェクトの出現
    for i = 1, 5, 1 do    
        tempTransform.scale.x = 0.2 + math.random() * 1.0
        tempTransform.scale.y = 0.2 + math.random() * 1.0
        tempTransform.scale.z = 0.2 + math.random() * 1.0
        tempTransform.translate = transform.translate
        tempTransform.translate.y = transform.translate.y + 1.0
        tempTransform.translate.z = transform.translate.z - 1.0
        tempTransform.rotate.x = 0.0
        tempTransform.rotate.y = math.random() * 6.28
        tempTransform.rotate.z = 0.0
        CreateEntity(hitEffectObjName,tempTransform)
    end

    -- 煙の出現
    for i = 1, maxNum, 1 do    
        tempTransform.scale.x = 1.0 + math.random() * 1.0
        tempTransform.scale.y = 1.0 + math.random() * 1.0
        tempTransform.scale.z = 1.0 + math.random() * 1.0
        tempTransform.translate = transform.translate
        tempTransform.translate.y = transform.translate.y + 1.0
        tempTransform.translate.z = transform.translate.z - 1.0
        tempTransform.rotate.x = 0.0
        tempTransform.rotate.y = math.random() * 6.28
        tempTransform.rotate.z = 0.0
        CreateEntity(smokeObjName,tempTransform)
    end

end

--[[
    更新処理
--]]
function Update()
    destroy()
end
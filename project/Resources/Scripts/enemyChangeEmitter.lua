changeObjName = "EnemyChangeParticle.json"

-- 生成する数
local maxNum = 10

--[[
    初期化処理
--]]
function Init()
    local tempTransform = Transform.new()

    -- 円の出現
    for i = 1, maxNum, 1 do    
        local scale = 0.2 + math.random() * 0.3
        tempTransform.scale.x = scale
        tempTransform.scale.y = scale
        tempTransform.scale.z = scale
        tempTransform.translate = transform.translate
        tempTransform.rotate.x = 0.9
        CreateEntity(changeObjName,tempTransform)
    end

end

--[[
    更新処理
--]]
function Update()
    destroy()
end
circleObjName = "EnemyCircleParticle.json"
coneObjName = "EnemyConeParticle.json"

-- 生成する数
local maxNum = 15

--[[
    初期化処理
--]]
function Init()
    local tempTransform = Transform.new()

    for i = 1, 3, 1 do    
    -- 円の演出を出現
    tempTransform.rotate.x = 0.9
    tempTransform.scale.x = 0.8
    tempTransform.scale.y = 0.8
    tempTransform.scale.z = 0.8
    tempTransform.translate = transform.translate
    tempTransform.translate.x = transform.translate.x + -0.5 + math.random() * 1.0
    tempTransform.translate.y = transform.translate.y + 1.0
    tempTransform.translate.z = transform.translate.z + -0.5 + math.random() * 1.0
    CreateEntity(circleObjName,tempTransform)
    end

    -- 破片の出現
    local tmp = 0.25
    local def = 0.1
    for i = 1, maxNum, 1 do    
        tempTransform.scale.x =  def + math.random() * tmp
        tempTransform.scale.y =  def + math.random() * tmp
        tempTransform.scale.z =  def + math.random() * tmp
        tempTransform.translate = transform.translate
        tempTransform.translate.y = transform.translate.y + 1.0
        tempTransform.rotate = transform.rotate
        CreateEntity(coneObjName,tempTransform)
    end

end

--[[
    更新処理
--]]
function Update()
    destroy()
end
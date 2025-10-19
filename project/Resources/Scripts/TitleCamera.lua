targetObjName = "ShotGunPlayer"
xOffset = 5.0
translateSpeed = 0.2 
local targetId = 0
local targetX = 0.0

function Init()
    targetId = GetEntity(targetObjName)
    targetX = 0.0
end

function Update()
    local tempX = GetTransform(targetId).translate.x

    if math.abs(transform.translate.x - tempX) >= xOffset then
        if transform.translate.x - tempX < 0.0 then
            targetX = targetX + xOffset*2.0
        else
            targetX = targetX - xOffset*2.0
        end
    end

    transform.translate.x = QFE.Math.SimpleEaseIn(transform.translate.x,targetX,translateSpeed)
end

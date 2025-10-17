objName = "ShotGunPlayer"

borderX = 10.0
maxBorderX = 10.0
borderSpeed = 1.0
local id = 0
local targetX = 0.0

function Init()
    borderX = maxBorderX
    id = GetEntity(objName)
    targetX = 0.0
end

function Update()
    local deltaTime = GetDeltaTime()
    local nowPlayerposX = GetTransform(id).translate.x
    transform.rotate.z = transform.rotate.z + deltaTime * 3.0

    borderX = borderX - (borderSpeed * deltaTime)

    -- プレイヤーが離れすぎたら瞬時に追いつく
    if  nowPlayerposX - transform.translate.x > maxBorderX then
        borderX = maxBorderX  
        targetX = nowPlayerposX 
    end

    -- 位置更新
    transform.translate.x = targetX- borderX
end

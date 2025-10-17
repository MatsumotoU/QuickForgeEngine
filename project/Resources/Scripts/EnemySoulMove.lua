maxAliveTime = 3.0
aliveTime = maxAliveTime
playerName = "ShotGunPlayer"
speed = 0.5
zSpeed = 0.5
local playerId = 0

function Init()
    aliveTime = maxAliveTime
    playerId = GetEntity(playerName)
    DebugLog("SpawnSoul")
end

function Update()
    local delta = GetDeltaTime()

    -- 寿命計算
    if aliveTime > 0.0 then
        aliveTime = aliveTime - delta
    else
        DebugLog("DestroySoul")
        destroy()
    end

    -- アニメーション
    local rotateSpeed = (aliveTime / maxAliveTime) * 10.0
    transform.rotate.x = transform.rotate.x + delta * rotateSpeed
    transform.rotate.y = transform.rotate.y + delta * rotateSpeed
    transform.rotate.z = transform.rotate.z + delta * rotateSpeed

    -- 追従処理
    local targetVector = Vector3.new()
    targetVector = GetTransform(playerId).translate - transform.translate
    targetVector = targetVector:Normalize()

    targetVector.x = 1.0

    targetVector.x = targetVector.x * speed * delta
    targetVector.y = targetVector.y * speed * delta
    targetVector.z = targetVector.z * speed * delta * zSpeed
    
    transform.translate = transform.translate + targetVector 
end

function OnCollisionStay(id,obj)
    if obj.tag == "player" then
        destroy()
    end
end
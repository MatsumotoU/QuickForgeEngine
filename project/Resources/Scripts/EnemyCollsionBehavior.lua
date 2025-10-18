
-- 生存フラグ
isAlive = true

-- カメラの名前
borderName = "DamageBorder"
soulName = "EnemySoul.json"
local targetId = 0
local targetTransform = Transform.new()


function Init()
    isAlive = true

    -- 追跡するidを取得
    targetId = GetEntity(borderName)
end

function Update()

    if not isAlive then 
        destroy()
    end

     -- カメラの追跡する位置を設定
    targetTransform = GetTransform(targetId)
    local targetX = targetTransform.translate.x

    --DebugLog("Id"..targetId)
    --DebugLog("cameraLeftBoderX"..targetX)
    --DebugLog("enemyPosX"..transform.translate.x)

    -- 画面左端を出たら生存フラグをfalse
    if transform.translate.x <= targetX then
        DebugLog("EnemyIsAliveFalse")
        if isAlive then
            isAlive = false
            CreateEntity(soulName,transform)
        end
        
    end
end

function OnCollisionEnter(id,obj)

    if obj.tag == "Bullet" then
        isAlive = false
    end
end

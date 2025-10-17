
-- 生存フラグ
isAlive = true

-- カメラの名前
cameraName = "name"
local targetId = 0
local targetTransform = Transform.new()
local offsetX = 5.0 -- カメラの画面端までの位置

function Init()
    isAlive = true

    -- 追跡するidを取得
    targetId = GetEntity(cameraName)
    -- カメラの画面端までの位置
    offsetX = 5.0
end

function Update()

    if not isAlive then 
        destroy()
    end

     -- カメラの追跡する位置を設定
    targetTransform = GetTransform(1)
    local targetX = targetTransform.translate.x - offsetX

    --DebugLog("Id"..targetId)
    --DebugLog("cameraLeftBoderX"..targetX)
    --DebugLog("enemyPosX"..transform.translate.x)

    -- 画面左端を出たら生存フラグをfalse
    if transform.translate.x <= targetX then
        DebugLog("EnemyIsAliveFalse")
        isAlive = false
    end
end

function OnCollisionEnter(id,obj)

    if obj.tag == "Bullet" then
        isAlive = false
    end
end

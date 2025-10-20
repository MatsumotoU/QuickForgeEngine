damageInterval = 0.0
maxDamageInterval = 0.5
hp = 5
maxHp = 5

local time = 0.0

stoneName = "hakaName"

-- 生存フラグ
isAlive = true
-- 敵のタイプ番号
enemyType = 0
-- 普通の敵の名前
normalEnemyName = "normalEnemy"

-- カメラの名前
cameraName = "Camera"
local targetId = 0
local targetTransform = Transform.new()
local offsetX = 12.0 -- カメラの画面端までの位置

function Init()
    isAlive = true
    hp = maxHp
    damageInterval = 0.0
    time = 0.0

     -- 追跡するidを取得
    targetId = GetEntity(cameraName)
    -- カメラの画面端までの位置
    offsetX = 12.0
end

function Update()

    if not isAlive then 
        destroy()
    end

    time = time + 1.0

    if damageInterval > 0.0 then
        damageInterval = damageInterval - 0.016
        transform.scale.y = math.sin(time) * math.sin(time)
    else
        transform.scale.y = 1.0
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
    if obj.tag == "bullet" then
        hp = hp - 1
        damageInterval = maxDamageInterval

        if hp <= 0 then
            if isAlive then
                isAlive = false
                if enemyType == 0 then
                    local temp = Transform.new()
                    temp.translate.x = transform.translate.x
                    temp.translate.y = transform.translate.y + 2.0
                    temp.translate.z = transform.translate.z
                    CreateEntity(stoneName,temp)
                elseif enemyType == 1 then 
                    -- 双子のゴーストの場合
                    local temp = Transform.new()
                    temp.translate.x = transform.translate.x
                    temp.translate.y = transform.translate.y + 2.0
                    temp.translate.z = transform.translate.z - 0.5
                    CreateEntity(normalEnemyName,temp)
                    temp.translate.z = transform.translate.z + 0.5
                    CreateEntity(normalEnemyName,temp)
                end
            end
        end
    end
end
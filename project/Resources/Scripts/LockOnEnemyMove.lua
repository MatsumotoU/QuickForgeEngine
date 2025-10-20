
LockOnObjName = "objName"

local id = 0

-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local nowSpeed = 0.0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5
end

--[[
    更新処理
--]]
function Update()

    id = GetEntity(LockOnObjName)

    if isHit then
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end

    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    targetTransform.translate.y = 0.0
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = targetTransform.translate.y - transform.translate.y
    dir.z = targetTransform.translate.z - transform.translate.z

    -- 距離を求める
    local distance = dir:Length()

    if distance > 2.0 then
        -- 正規化する
        dir:Normalize()
        local deltaTime = GetDeltaTime()
        -- 移動量を取得
        move.x = dir.x * nowSpeed * deltaTime
        move.y = dir.y * nowSpeed * deltaTime
        move.z = dir.z * nowSpeed * deltaTime
        -- 移動
        transform.translate.x = transform.translate.x + move.x
        transform.translate.y = transform.translate.y + move.y
        transform.translate.z = transform.translate.z + move.z
    else
        -- 移動
        transform.translate.x = transform.translate.x + move.x
        transform.translate.y = transform.translate.y + move.y
        transform.translate.z = transform.translate.z + move.z
    end

    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

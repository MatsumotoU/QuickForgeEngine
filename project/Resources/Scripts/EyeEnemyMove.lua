LockOnObjName = "ShotGunPlayer"

local id = 0

-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

-- 向いているベクトル
myDir = Vector3.new(0.0,0.0,0.0)

-- 視野角
local viewAngle = math.rad(45.0)

--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5
-- 自分の向いているベクトルを設定
myDir.x = -1.0
myDir.y = 0.0
myDir.z = 0.0
end

--[[
    更新処理
--]]
function Update()

    if isHit then
        moveSpeed = slowSpeed
    else
        moveSpeed = speed
    end

    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    -- プレイヤーへの方向を求める
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = targetTransform.translate.y - transform.translate.y
    dir.z = targetTransform.translate.z - transform.translate.z
    -- 正規化する
    dir:Normalize()

    -- 内積を求める
    local dot = myDir.x * dir.x + myDir.y * dir.y + myDir.z * dir.z
    dot = math.max(-1.0,math.min(1.0,dot)) -- 範囲を制限する
    local angle = math.acos(dot)

    -- プレイヤーが視界にいる時、移動する
    if angle <= viewAngle and angle > 0.0 then
        local deltaTime = GetDeltaTime()
        transform.translate.x = transform.translate.x + dir.x * moveSpeed * deltaTime
        transform.translate.y = transform.translate.y + dir.y * moveSpeed * deltaTime
        transform.translate.z = transform.translate.z + dir.z * moveSpeed * deltaTime
    end

    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end
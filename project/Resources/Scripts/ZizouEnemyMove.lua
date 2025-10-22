LockOnObjName = "ShotGunPlayer"

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

-- 向いているベクトル
myDir = Vector3.new(0.0,0.0,0.0)

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
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end

    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    local playerDir = Vector3.new(0.0,0.0,0.0)
    local rotateY = targetTransform.rotate.x
    rotateY = math.rad(rotateY)
    playerDir = Vector3.new(math.sin(rotateY),0.0,math.cos(rotateY))

     -- 正規化する
    playerDir:Normalize()

    -- 内積を求める
    local dot = myDir.x * playerDir.x + myDir.y * playerDir.y + myDir.z * playerDir.z
    dot = math.max(-1.0,math.min(1.0,dot)) -- 範囲を制限する

    -- 速度を求める
    local angleFactor = 1.0 - ((dot + 1.0) / 2.0)
    local finalSpeed = nowSpeed * (1.0 + angleFactor)
    local deltaTime = GetDeltaTime()
    finalSpeed = finalSpeed * deltaTime

    -- プレイヤーへの方向を求める
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = targetTransform.translate.y - transform.translate.y
    dir.z = targetTransform.translate.z - transform.translate.z
    -- 正規化する
    dir:Normalize()

    -- 移動処理
    transform.translate.x = transform.translate.x + dir.x * finalSpeed
    transform.translate.y = transform.translate.y + dir.y * finalSpeed
    transform.translate.z = transform.translate.z + dir.z * finalSpeed

    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end
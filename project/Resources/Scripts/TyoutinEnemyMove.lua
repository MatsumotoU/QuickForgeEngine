-- プレイヤー
PlayerObjName = "ShotGunPlayer"

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

-- 移動距離
local moveDistance = 0.0

-- 弾の発射フラグ
local shotBullet = false

--[[
    初期化処理
--]]
function Init()
id = GetEntity(PlayerObjName)
DebugLog("PlayerObjNameID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5
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

    -- 弾の発射を取得
    if QFE.Input.GetKeyTrigger("Shot") then
        if not shotBullet then
            shotBullet = true
            moveDistance = 0.0
        end
    end

    if shotBullet then
        -- 移動
        local timer = GetDeltaTime()
        moveDistance = moveDistance + moveSpeed * timer
        transform.translate.x = transform.translate.x + moveSpeed * timer

        -- 3マス分移動したら終了
        if moveDistance >= 3.0 then
            shotBullet = false
        end
    end
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

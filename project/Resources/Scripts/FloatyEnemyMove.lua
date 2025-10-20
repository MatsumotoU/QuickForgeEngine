
-- 速度
speed = 0.0
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local nowSpeed = 0.0

-- ヒットフラグ
local isHit = false

function Init()
DebugLog("Init")
-- スロー速度を求める
slowSpeed = speed * 0.5
end

function Update()
    if isHit then
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end

    local deltaTime = GetDeltaTime()

    -- 移動
    transform.translate.x = transform.translate.x - nowSpeed * deltaTime

    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end
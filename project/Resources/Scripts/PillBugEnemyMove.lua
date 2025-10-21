-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- ヒットフラグ
local isHit = false

-- 移動する時間(秒)
moveTime = 1.0
-- 止まる時間
stopTime = 2.0
local timer = 0.0

--[[
    初期化処理
--]]
function Init()
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

    local t = GetDeltaTime()
    timer = timer + t

    -- 移動する
    if timer <= moveTime then
        transform.translate.x = transform.translate.x - (moveSpeed * t)
    end

    -- 止まる時間が過ぎたら
    if timer >= moveTime + stopTime then
        timer = 0.0
    end

     isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

-- 速度
speed = 0.0
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- ヒットフラグ
local isHit = false

-- 角度
local angleRightY = 0.0
local angleLeftY = 0.0

--[[
    初期化処理
--]]
function Init()
-- スロー速度を求める
slowSpeed = speed * 0.5

-- 斜め45度を求める
angleRightY = -math.pi / 4.0
angleLeftY = -math.pi * 2.0 / 4.0
-- 向いている方向を設定
transform.rotate.y = angleRightY
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

    -- 移動
    transform:AddForward(moveSpeed)

    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

function OnCollisionStay(id,obj)
    -- 壁に触れた場合向く角度を変える
    if obj.tag == "Wall" then
        if transform.rotate.y == angleRightY then
            transform.rotate.y = angleLeftY
        else
            transform.rotate.y = angleRightY
        end
    end
end
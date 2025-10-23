
-- 速度
speed = 0.0
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- ヒットフラグ
local isHit = false

local angleY = 0.0

local isStart = false

local rotateType = 0

-- 当たり判定のためのインターバル
local timer = 0.0
local intervalTime = 1.0

--[[
    初期化処理
--]]
function Init()
-- スロー速度を求める
slowSpeed = speed * 0.5
angleY = math.pi / 4.0
end

--[[
    更新処理
--]]
function Update()

    if not isStart then
        isStart = true
        transform.rotate.y = -angleY
    end

    if isHit then
        moveSpeed = slowSpeed
    else
        moveSpeed = speed
    end

    -- 移動
    local deltaTime = GetDeltaTime()
    transform:AddForward(moveSpeed * deltaTime)

    if timer <= 1.0 then
        timer = timer + deltaTime
    end

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
        if timer >= 1.0 then
            timer = 0.0
            if rotateType == 0 then
                rotateType = 1
                transform.rotate.y = -angleY * 3.0
            else
                rotateType = 0
                transform.rotate.y = -angleY
            end
        end
    end
end
local isAddForce = false

local timer = 0.0
local maxTime = 1.5

local scaleMove = 0.0

local startScale = 0.0

--[[
    初期化処理
--]]
function Init()
    isAddForce = false
end

--[[
    更新処理
--]]
function Update()
    if not isAddForce then
        isAddForce = true

        -- プレイヤーが向いている方向から速度を求める
        local yaw = transform.rotate.y
        local forward = Vector3.new()
        forward.x = math.cos(0.0) * math.sin(yaw)
        forward.y = math.sin(0.0)
        forward.z = math.cos(0.0) * math.cos(yaw)
        force.velocity.x = -forward.x * 3.0 
        force.velocity.y = -2.0 + math.random() * 4.0 
        force.velocity.z = -forward.z * 3.0

        -- 角度を求める
        transform.rotate.x = math.random() * 6.28
        transform.rotate.y = math.random() * 6.28
        transform.rotate.z = math.random() * 6.28

        -- 位置を求める
        transform.translate.x = transform.translate.x + math.random() * -2.0
        transform.translate.y = transform.translate.y + -1.0 + math.random() * 2.0
        transform.translate.z = transform.translate.z + -1.5 + math.random() * 3.0

        -- 最初の大きさを保存
        startScale = transform.scale.x
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    scaleMove = QFE.Math.Leap(0.0,startScale,EaseIn(t))

    -- 拡縮
    transform.scale.x = scaleMove
    transform.scale.y = scaleMove
    transform.scale.z = scaleMove

    if timer >= maxTime then
        destroy()
    end

end

function EaseIn(t)
    return t * t * t
end

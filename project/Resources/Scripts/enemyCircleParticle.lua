local isAddForce = false

local timer = 0.0
local maxTime = 0.2

local scaleMove = 0.0

function Init()
    isAddForce = false
end

function Update()
    if not isAddForce then
        isAddForce = true
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    scaleMove = QFE.Math.Leap(0.3,0.8,EaseIn(t))

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
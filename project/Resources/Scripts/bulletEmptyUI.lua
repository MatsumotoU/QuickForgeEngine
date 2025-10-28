
local timer = 0.0
local maxTime = 1.0

function Init()

end

function Update()

     local deltatime = GetDeltaTime()
    timer = timer + deltatime

     -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    local scale = QFE.Math.Leap(0.0,1.0,EaseIn(t))

    transform.scale.x = scale
    transform.scale.y = scale
    transform.scale.z = scale

    if timer >= maxTime then
        destroy()
    end
end

function EaseIn(t)
    return t * t * t * t
end

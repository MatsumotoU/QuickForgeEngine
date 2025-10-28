
local timer = 0.0
local maxTime = 1.0

local mat = Material.new()

function Init()
    mat = GetMaterial(GetThisEntityId())
end

function Update()

    if timer == 0.0 then
        transform.scale.x = 1.0
        transform.scale.z = 0.1
        transform.translate.y = transform.translate.y + 1.0
        transform.translate.z = transform.translate.z + 0.5
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    if t <= 0.25 then

        local widthTimer = t / 0.25

        transform.scale.x = QFE.Math.Leap(0.1,1.0,EaseIn(widthTimer))
        transform.scale.z = QFE.Math.Leap(1.0,0.1,EaseIn(widthTimer))

    elseif t <= 0.5 then

        local heightTimer = (t - 0.25) / 0.25

        transform.scale.x = QFE.Math.Leap(0.8,0.1,EaseOut(heightTimer))
        transform.scale.z = QFE.Math.Leap(0.8,1.0,EaseOut(heightTimer))


    elseif t <= 0.75 then
  
        local normalTimer = (t - 0.5) / 0.25

        mat.color.w  = math.abs(math.sin(normalTimer * math.pi * 2.0 / (0.25 / 10.0)))

    else
        local alphaTimer = (t - 0.75) / 0.25

        mat.color.w  = QFE.Math.Leap(0.0,1.0,alphaTimer)
    end

end

function EaseIn(t)
    return t * t * t
end

function EaseOut(t)
    return -t * (t - 2.0)
end
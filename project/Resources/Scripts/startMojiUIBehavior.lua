
local timer = 0.0
local maxTime = 1.5

local circleId = 0
local mojiId = 0

function Init()
    local tmpTransform = Transform.new()
    -- 円を生成
    tmpTransform.translate.x = 640.0
    tmpTransform.translate.y = 240.0
    circleId = CreateEntity("RedCircleUI.json",tmpTransform)

    -- 文字を生成
    tmpTransform.translate.y = 237.0
    mojiId = CreateEntity("LeftMoveGuideUI.json",tmpTransform)
end

function Update()

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    local cir = GetTransform(circleId)
    local moj = GetTransform(mojiId)

    if t <= 0.25 then

        local widthTimer = t / 0.25

        cir.scale.x = QFE.Math.Leap(0.1,0.5,EaseIn(widthTimer))
        cir.scale.y = QFE.Math.Leap(0.6,0.4,EaseIn(widthTimer))

        moj.scale.x = QFE.Math.Leap(0.5,0.0,EaseIn(widthTimer))
        moj.scale.y = QFE.Math.Leap(0.2,0.0,EaseIn(widthTimer))

    elseif t <= 0.5 then

        local heightTimer = (t - 0.25) / 0.25

        cir.scale.x = QFE.Math.Leap(1.4,0.1,EaseOut(heightTimer))
        cir.scale.y = QFE.Math.Leap(0.4,0.6,EaseOut(heightTimer))

        moj.scale.x = QFE.Math.Leap(1.0,0.5,EaseOut(heightTimer))
        moj.scale.y = QFE.Math.Leap(1.0,0.2,EaseOut(heightTimer))

    elseif t <= 0.75 then

    else
        local alphaTimer = (t - 0.75) / 0.25

        cir.scale.y = QFE.Math.Leap(0.0,0.4,EaseOut(alphaTimer))

        moj.scale.y = QFE.Math.Leap(0.0,1.0,EaseOut(alphaTimer))

    end

    if timer >= maxTime then
        destroy()
    end

end

function EaseIn(t)
    return t * t * t
end

function EaseOut(t)
    return -t * (t - 2.0)
end

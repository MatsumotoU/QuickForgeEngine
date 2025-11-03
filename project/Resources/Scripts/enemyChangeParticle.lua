local isAddForce = false

local timer = 0.0
local maxTime = 2.0

local scaleMove = 0.0
local startScale = 0.0

local rotateMove = 0.0

local radius = 1.0

local centerX = 0.0
local centerZ = 0.0

local startRadius = 0.0
local endRadius = 0.0

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

        startScale = transform.scale.x

        centerX = transform.translate.x
        centerZ = transform.translate.z

        rotateMove = math.random() * 3.2

        startRadius = 0.2 + math.random() * 0.4
        endRadius = 1.0 + math.random() * 0.5
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    radius = QFE.Math.Leap(endRadius,startRadius,t)

    rotateMove = rotateMove + 8.0 * deltatime

    -- 回転運動
    transform.translate.x = centerX + math.cos(rotateMove) * radius
    transform.translate.z = centerZ + math.sin(rotateMove) * radius

    transform.translate.y = transform.translate.y + 0.5 * deltatime

    scaleMove = QFE.Math.Leap(0.4,startScale,t)

    -- 拡縮
    transform.scale.x = scaleMove
    transform.scale.y = scaleMove
    transform.scale.z = scaleMove

    if timer >= maxTime then
        destroy()
    end

end
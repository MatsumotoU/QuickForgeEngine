
local waitTimer = 0.0
local waitMaxTime = 10.0

local gaugeId = 0

local playerId = 0
isMax = false

local maxGaugeNum = 1
gaugeNum = 0

function Init()
    gaugeId = GetEntity("RedGauge")

    playerId = GetEntity("ShotGunPlayer")
    gaugeNum = 0
end

function Update()

    if gaugeNum > maxGaugeNum then
        gaugeNum = maxGaugeNum

    elseif gaugeNum < 0 then
        gaugeNum = 0
    end

    if gaugeNum >= maxGaugeNum then
        isMax = true
    else
        isMax = false

    end


    local tmpTransform = GetTransform(gaugeId)
    
    tmpTransform.scale.x = (gaugeNum / maxGaugeNum) * 1.5

    local playerTmp = GetTransform(playerId)
    transform.translate = playerTmp.translate
    transform.translate.y = playerTmp.translate.y + 1.5
end

function AddNum()
    gaugeNum = gaugeNum + 1
end

function DeleteNum()
    gaugeNum = 0
end

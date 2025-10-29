
local waitTimer = 0.0
local waitMaxTime = 10.0

local gaugeId = 0

local playerId = 0

gaugeNum = 0

function Init()
    gaugeId = GetEntity("RedGauge")

    playerId = GetEntity("ShotGunPlayer")
    gaugeNum = 0
end

function Update()

    if gaugeNum > 4 then
        gaugeNum = 4
    elseif gaugeNum < 0 then
        gaugeNum = 0
    end

    local tmpTransform = GetTransform(gaugeId)
    
    tmpTransform.scale.x = (gaugeNum / 4.0) * 1.5

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

local ballIds = {}

local function RemoveInactiveBalls()
    local activeBallIds = {}
    for _, ballId in ipairs(ballIds) do
        if GetEntityTag(ballId) == "ball" then
            activeBallIds[#activeBallIds + 1] = ballId
        end
    end
    ballIds = activeBallIds
end

function Init()
    ballIds = {}
    ballIds[1] = SimpleCreateEntity("Ball.json")
    ResetBallPos()
end

function Update()
    if CountEntityTag("card") > 0 or not QFE.Input.GetKeyTrigger("Jump") then
        return
    end

    RemoveInactiveBalls()
    for _, ballId in ipairs(ballIds) do
        RunEntityScriptFunction(ballId, "Ball.lua", "StartBall")
    end
end

function OnUpGradeBallCard()
    RemoveInactiveBalls()
    ballIds[#ballIds+1] = SimpleCreateEntity("Ball.json")
    ResetBallPos()
end

function OnNextStage()
    ResetBallPos()
end

function ResetBallPos()
    RemoveInactiveBalls()
    local v = Vector3.new()
    local offset = 1.2
    v.x = (-offset * 0.5) *  (#ballIds - 1.0)
    v.z = -3.0

    for i = 1, #ballIds, 1 do
        SetTranslate(ballIds[i],v)
        v.x = v.x + offset
    end
end

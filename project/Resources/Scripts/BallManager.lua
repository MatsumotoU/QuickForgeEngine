local ballIds = {}

function Init()
    ballIds[1] = SimpleCreateEntity("Ball.json")
end

function Update()

end

function OnUpGradeBallCard()
    ballIds[#ballIds+1] = SimpleCreateEntity("Ball.json")
    ResetBallPos()
end

function OnNextStage()
    ResetBallPos()
end

function ResetBallPos()
    local v = Vector3.new()
    local offset = 1.2
    v.x = (-offset * 0.5) *  (#ballIds - 1.0)

    for i = 1, #ballIds, 1 do
        
        v.z = -3.5
        
        SetTranslate(ballIds[i],v)
        v.x = v.x + offset
    end
end
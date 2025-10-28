
aliveScriptName = "EnemyHp.lua"
isAliveName = "isAlive"

performingTime = 1.0
moveAnimCount = 0.0



function Init()

moveAnimCount = 0.0

end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function Update()

    -- アタッチ対象のid
    local thisId = this.GetEntityId()
    local isAlive = GetEntityScriptGlobal(thisId,aliveScriptName,isAliveName)

    if  isAlive == true then
    GhostMoveAnimUpdate(performingTime)
    end

end

function GhostMoveAnimUpdate(kPerformingTime_)

    local inv_performTime = 1.0 / kPerformingTime_
    moveAnimCount = moveAnimCount + 0.0166666 * inv_performTime

    local delta = math.sin(math.pi*moveAnimCount)

    local default = 0.75
    local conv1 = delta*0.5
    transform.translate.y = default + conv1 

    local defaultScale = 1.0
    local conv1 = delta*0.125
    transform.scale.x = defaultScale+conv1 
    transform.scale.y = defaultScale+conv1 
    transform.scale.z = defaultScale+conv1 


    return false

end


function EaseInExpo(st_, end_, t_)

    local tmp = (10.0 * t_) - 10.0
    local convertedT = 2.0^tmp

    return Lerp(st_, end_, convertedT)

end

function EasingInBack(st_, end_, t_)
    local c1 = 1.70158
    local c3 = c1 + 1.0

    local convertedT = c3 * t_ * t_ * t_ - c1 * t_ * t_

    return Lerp(st_, end_, convertedT)
end

function EaseOutCubic(st_, end_, t_)
	
    local tmp = 1.0 - t_
    local convertedT = 1.0 - tmp^3.0

    return Lerp(st_, end_, convertedT)
end	
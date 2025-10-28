
aliveScriptName = "EnemyHp.lua"
isAliveName = "isAlive"

local performingTime = 0.25
deathAnimCount = 0.0
deathAnimType = 0


function Init()

deathAnimCount = 0.0

end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function Update()

    -- アタッチ対象のid
    local thisId = this.GetEntityId()
    local isAlive = GetEntityScriptGlobal(thisId,aliveScriptName,isAliveName)

    if  isAlive == false then

        local isDeathAnimEnd = false

        if deathAnimType == 0 then
            isDeathAnimEnd = GhostDeathAnimUpdate(performingTime) 
        else
            isDeathAnimEnd = GhostDeathAnimUpdate(performingTime) 
        end

        if isDeathAnimEnd == true then
           destroy()

        end
    end

end

function GhostDeathAnimUpdate(kPerformingTime_)
    local dst_scaleX = 0.2
    local st_scaleX = dst_scaleX
    local dst_alpha = 0.0
    local mat = GetMaterial(GetThisEntityId())
    local rotateSum = 2.0 * 2.0 


    local inv_performTime = 1.0 / kPerformingTime_
    deathAnimCount = deathAnimCount + 0.0166666 * inv_performTime

    mat.color.w = EaseOutCubic(1.0, dst_alpha, deathAnimCount)

    if deathAnimCount >= 0.0 and deathAnimCount < 0.5 then
    local t = deathAnimCount * 2.0

    transform.scale.x = EasingInBack(1.0, dst_scaleX, t) 

    else
        local t = (deathAnimCount - 0.5 ) * 2.0
        local dst_rotateRad = 3.14159265359 * rotateSum  * t 

        transform.scale.y = EaseInExpo(1.0, 0.0, t) 

        transform.scale.x = Lerp(st_scaleX , 0.0, t) 

        transform.rotate.y = dst_rotateRad 

    end


    if deathAnimCount >= 1.0 then

        deathAnimCount = 0.0
        return true

    end

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



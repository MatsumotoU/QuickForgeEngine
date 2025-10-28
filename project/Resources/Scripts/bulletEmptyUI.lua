
local timer = 0.0
local maxTime = 1.0

local performCnt = 0.0
local performTime = 2.0
local inv_performTime = 0.0

local st_posY = 0.0
local onlyOnce = true 
local add_posY = 1.25 


function Init()
inv_performTime = 1.0 / performTime
end

function Update()

    if onlyOnce == true then
        onlyOnce = false
        st_posY = transform.translate.y 
        
    end

     local deltatime = GetDeltaTime()
    -- timer = timer + deltatime

    --  -- 0から1の範囲に収める
    -- local t = math.min(timer / maxTime,1.0)

    -- local scale = QFE.Math.Leap(0.0,1.0,EaseIn(t))

    -- transform.scale.x = scale
    -- transform.scale.y = scale
    -- transform.scale.z = scale

    performCnt = performCnt+ inv_performTime * deltatime


    transform.translate.y = EaseInExpo(st_posY,st_posY+ add_posY,performCnt) 

    local mat = GetMaterial(GetThisEntityId())

    if performCnt <= 0.3 then
        local t = performCnt * 3.333333333 
        local dst_scale = 0.75
        local dst_alpha = 0.75

        transform.scale.x =  EaseOutBack(0.0,dst_scale,t) 
        transform.scale.y =  EaseOutBack(0.0,dst_scale,t) 
        mat.color.w = EaseOutCubic(0.0,dst_alpha,t)

    elseif  performCnt > 0.3 and performCnt <= 0.8 then
        local t = (performCnt - 0.3) * 2.0 
        local add_alpha = 0.25
        local tmp = 0.5 + math.sin(t*math.pi*6.0) * add_alpha 
        mat.color.w = tmp

    else
        local dst_alpha = 0.0
        local t = (performCnt - 0.8) * 5.0 

        mat.color.w = EaseInBounce(1.0,dst_alpha,t)

    end

    if performCnt >= 1.0 then
        destroy()
    end
end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end


function EaseIn(t)
    return t * t * t * t
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


function EaseOutBounceForConvert(t_)

    	local n1 = 7.5625
		local d1 = 2.75

		local convertedT = 0.0

		if t_ < (1.0 / d1) then
		
			convertedT = n1 * t_ * t_

		elseif t_ < (2.0 / d1) then
		
            t_ = t_ - (1.5 / d1)
			convertedT = n1 * t_ * t_ + 0.75
		
		elseif t_ < (2.5 / d1) then

		    t_ = t_ - (2.25 / d1)
			convertedT = n1 * t_ * t_ + 0.9375

        else
			t_ = t_ - (2.625 / d1)
			convertedT = n1 * t_ * t_ + 0.984375
		
        end

    return convertedT
end


function EaseInBounce(st_, end_, t_)
    return Lerp(st_,end_,1.0 - EaseOutBounceForConvert(1.0 - t_))
end

function EaseOutBack(st_, end_, t_)
    local c1 = 1.70158
    local c3 = c1 + 1.0

    local convertedT = 1.0 + c3 * (t_ - 1.0)*(t_ - 1.0)*(t_ - 1.0) + c1 * (t_ - 1.0) * (t_ - 1.0);

    return Lerp(st_, end_, convertedT)
end

    
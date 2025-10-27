-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- ヒットフラグ
local isHit = false

-- 移動する時間(秒)
moveTime = 1.0
local inv_moveTime = 0.0


local cur_act = 1
actCnt = 0.0
delta_xPos = 5.0
local next_xPos = 0.0
local cur_xPos = 0.0
local thetaForidle = 0.0

--[[
    初期化処理
--]]
function Init()
-- スロー速度を求める
slowSpeed = speed * 0.5
inv_moveTime = 1.0 / moveTime
end

--[[
    更新処理
--]]
function Update()

    -- 回転
    transform.rotate.y = -math.pi*0.5

    if isHit then
        moveSpeed = slowSpeed
    else
        moveSpeed = speed
    end

    local deltaTime = GetDeltaTime()
    

    if actCnt <= 1.0 then
            -- 移動する
        if cur_act == 0 then
            local dst_rotateX = math.pi*2.5

            actCnt = actCnt + (inv_moveTime * deltaTime)
            transform.rotate.x = EaseInBounce(0.0,dst_rotateX,actCnt) 
            transform.translate.x = EaseInBounce(cur_xPos , next_xPos,actCnt) 
        else
            local add_scale = 0.065
            thetaForidle = thetaForidle + math.pi * deltaTime*0.75
            local dstScale = 1.0 + math.sin(thetaForidle*0.5)*math.cos(thetaForidle*3.0) * add_scale

            transform.scale.x = dstScale
            transform.scale.y = dstScale
            transform.scale.z = dstScale


        end

    else
        cur_act = -1
        actCnt = 0.0

    end

     isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true

    elseif obj.tag == "Wall" then
        cur_act = 1

    elseif obj.tag == "Tombstone" then
        cur_act = 1

    elseif obj.tag == "bullet" then

        if cur_act ~= 0 then
        next_xPos = transform.translate.x - delta_xPos
        cur_xPos = transform.translate.x
        cur_act = 0
        end
        
    end

end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function EasingInBack(st_, end_, t_)
    local c1 = 1.70158
    local c3 = c1 + 1.0

    local convertedT = c3 * t_ * t_ * t_ - c1 * t_ * t_

    return Lerp(st_, end_, convertedT)
end

function EaseOutBounce(st_, end_, t_)

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




		
		-- 	convertedT = n1 * (t_ -= 1.5f / d1 )* t_ +0.75f;
		
		-- elseif t_ < (2.5 / d1) then
		
		-- 	convertedT = n1 * (t_ -= 2.25f / d1) * t_ + 0.9375f;

        -- else
		
		-- 	convertedT = n1 * (t_ -= 2.625f / d1) * t_ + 0.984375f;

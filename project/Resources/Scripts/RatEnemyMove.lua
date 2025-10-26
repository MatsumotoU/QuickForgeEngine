
-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- ヒットフラグ
local isHit = false
local wallHit = false

actCounter = 0.0
actStatus = true
local next_actStatus = 0

-- 移動する時間(秒)
moveTime = 1.0
local inv_moveTime = 1.0 / moveTime

-- 止まる時間
stopTime = 2.0
local inv_stopTime= 1.0 / stopTime

--次の移動角度
local nextMoveAngle = 0.0
local maxAddRad = math.pi * 0.8
local defaultRad =  -math.pi * 0.1
nextMoveAngle = defaultRad 
local currentRad = nextMoveAngle  


--[[
    初期化処理
--]]
function Init()
-- スロー速度を求める
slowSpeed = speed * 0.5
transform.rotate.y = nextMoveAngle
nextMoveAngle = defaultRad 
inv_stopTime= 1.0 / stopTime
inv_moveTime = 1.0 / moveTime
end

--[[
    更新処理
--]]
function Update()    

    local deltaTime = GetDeltaTime()

    if isHit then
        moveSpeed = slowSpeed

    else

        moveSpeed = speed
    end

     -- 移動する
    if actStatus == 0 then 
        local deltaRotate = math.pi / 8.0
        local numRepeat = 8

        next_actStatus = 1
        actCounter  = actCounter  + deltaTime * inv_moveTime
        transform:AddForward(moveSpeed * deltaTime)
        
        local cov = math.sin(math.pi * actCounter*numRepeat)*deltaRotate
        transform.rotate.y = currentRad + cov 



        
     -- 止まる
    else
        next_actStatus = 0
        actCounter  = actCounter  + deltaTime * inv_stopTime
        transform.rotate.y = EaseOutCubic(currentRad,nextMoveAngle,actCounter)


    end


    if actCounter >= 1.0 then
        inv_moveTime = 1.0 / moveTime
        actStatus = next_actStatus  
        currentRad  = transform.rotate.y
        actCounter = 0.0
        nextMoveAngle = defaultRad + (-maxAddRad * math.random())

    end

     isHit = false
     wallHit  = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
        
    elseif obj.tag == "Wall"  then

        wallHit = true
    end



end


function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function EaseOutCubic(st_, end_, t_)
	
    local tmp = 1.0 - t_
    local convertedT = 1.0 - tmp^3.0

    return Lerp(st_, end_, convertedT)
end	
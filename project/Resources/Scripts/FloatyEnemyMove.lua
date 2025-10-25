
-- 速度
speed = 0.0
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local nowSpeed = 0.0

local nextRotate = math.pi*0.5
rotateCount = 0.0
rotateTime = 1.0

wallHitCnt = 0
-- ヒットフラグ
local isHit = false


function Init()
DebugLog("Init")
-- スロー速度を求める
slowSpeed = speed * 0.5
rotateCount = 1.1
local isWallHit = false
local isWallHitbuff = false
-- transform.scale.x = 0.5
-- transform.scale.y = 0.5
-- transform.scale.z = 0.5
nextRotate = math.pi
end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function Update()
    local deltaTime = GetDeltaTime()

    if isWallHit == true then
        isWallHitbuff  = true 
    end

    if isWallHitbuff == true then
        wallHitCnt = wallHitCnt + 1 

        if wallHitCnt >= 150 then
            wallHitCnt = 0
            isWallHitbuff = false
        end
    end

    if isWallHit == true and wallHitCnt == 1 then
        nextRotate = nextRotate + math.pi
        rotateCount = 0.0
    end

    if rotateCount <= 1.0 then

        local inv_rotateTime = 1.0 / rotateTime
        rotateCount = rotateCount + deltaTime * inv_rotateTime

        transform.rotate.y = EaseOutBack(nextRotate- math.pi ,nextRotate,rotateCount)
    end

    if  rotateCount > 1.0 then
        -- 移動
        --transform.translate.z = transform.translate.z + speed*deltaTime
        transform:AddForward(-speed*deltaTime)

    end

    if isHit then
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end


    isHit = false
    isWallHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

function OnCollisionStay(id,obj)

    if obj.tag == "Wall" then
        isWallHit = true 
        -- if isWallHitbuff == false then
        --        isWallHit = true     
        -- end
    end
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

function EaseOutBack(st_, end_, t_)
	
    local c1 = 1.70158
    local c3 = c1 + 1.0

    local tmp = t_ - 1.0

    local convertedT = 1.0 + (c3 *tmp *tmp *tmp)  + (c1 * tmp *tmp) 

    return Lerp(st_, end_, convertedT)
end	


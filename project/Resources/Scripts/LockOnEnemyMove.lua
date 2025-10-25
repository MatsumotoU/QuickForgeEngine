
LockOnObjName = "objName"

local id = 0

-- 速度
speed = 0.04
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local nowSpeed = 0.0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

local myDir = Vector3.new(-1.0,0.0,0.0)
targetRate = 0.7 
maxSpeed = 5.0
moveAcceleraion = 10.0
local cur_velocity = Vector3.new(0.0,0.0,0.0)

local isObstacleHit = false
local hitCoolTimeCnt = 0.0
hitCoolTime = 0.5
local inv_hitCoolTime = 0.0



-- ヒットフラグ
local isHit = false

--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5
inv_hitCoolTime = 1.0 / hitCoolTime 
end

--[[
    更新処理
--]]
function Update()

    local deltaTime = GetDeltaTime()

    if isHit then
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end

    if hitCoolTimeCnt < 1.0 then
        hitCoolTimeCnt = hitCoolTimeCnt+ inv_hitCoolTime *deltaTime
    end


    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    targetTransform.translate.y = 0.0
    local targetDir = Vector3.new(0.0,0.0,0.0)
    local adjustPosToLittleFar = 1.25 

    local e_targetPos = Vector3.new(targetTransform.translate.x,targetTransform.translate.y,targetTransform.translate.z)
    e_targetPos = e_targetPos:Normalize()
    
    targetDir.x = targetTransform.translate.x - transform.translate.x
    targetDir.y = 0.0
    targetDir.z = targetTransform.translate.z - transform.translate.z
    
    myDir = myDir:Normalize()
    targetDir = targetDir:Normalize();

    --57.2958f
    local numCos =  myDir.x * targetDir.x + myDir.y * targetDir.y + myDir.z * targetDir.z
    local numArcCos = math.acos(numCos)
    local tmp = math.sin(numArcCos) 

    if tmp  >= 0.00001 then
        local len = math.sin(numArcCos * (1.0 - targetRate )) /  tmp 
        local len2 = math.sin(numArcCos*targetRate) / tmp 
        
        local buff_myDir = Vector3.new(myDir.x,myDir.y,myDir.z)
        buff_myDir.x = myDir.x * len 
        buff_myDir.y = myDir.y * len
        buff_myDir.z = myDir.z * len

        local buff_targetDir = Vector3.new(targetDir.x,targetDir.y,targetDir.z)
        buff_targetDir.x = targetDir.x * len2 
        buff_targetDir.y = targetDir.y * len2
        buff_targetDir.z = targetDir.z * len2

        myDir = buff_myDir + buff_targetDir

    end

    local angle = math.atan(myDir.x, myDir.z)
    transform.rotate.y = angle
    
    cur_velocity.x = cur_velocity.x + myDir.x * moveAcceleraion
    cur_velocity.z = cur_velocity.z + myDir.z * moveAcceleraion

    if cur_velocity.x > maxSpeed then
         cur_velocity.x = maxSpeed

    elseif cur_velocity.x < -maxSpeed then
         cur_velocity.x = -maxSpeed
    end


    if cur_velocity.z > maxSpeed then
         cur_velocity.z = maxSpeed

    elseif cur_velocity.z < -maxSpeed then
         cur_velocity.z = -maxSpeed
    end

    if isObstacleHit == true then
        local decCoe = 0.25
        cur_velocity.x = cur_velocity.x * decCoe 
        cur_velocity.z = cur_velocity.z * decCoe 

    end

    transform.translate.x = transform.translate.x + cur_velocity.x*deltaTime
    transform.translate.z = transform.translate.z + cur_velocity.z*deltaTime



    isHit = false
    isObstacleHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    elseif obj.tag == "Wall" or obj.tag == "Tombstone" then

        if hitCoolTimeCnt >= 1.0 then

            isObstacleHit = true 
            hitCoolTimeCnt = 0.0

        end

    end

end



function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end





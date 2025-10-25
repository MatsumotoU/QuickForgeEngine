LockOnObjName = "ShotGunPlayer"

local id = 0

-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local nowSpeed = 0.0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

-- 向いているベクトル
myDir = Vector3.new(0.0,0.0,0.0)

--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5
-- 自分の向いているベクトルを設定
myDir.x = -1.0
myDir.y = 0.0
myDir.z = 0.0
end

--[[
    更新処理
--]]
function Update()

    if isHit then
        nowSpeed = slowSpeed
    else
        nowSpeed = speed
    end

    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    local playerDir = Vector3.new(0.0,0.0,0.0)
    local rotateY = targetTransform.rotate.y
    rotateY = math.rad(rotateY)
    playerDir = Vector3.new(math.cos(rotateY),0.0,math.sin(rotateY))

     -- 正規化する
    playerDir:Normalize()

    -- 内積を求める
    local dot = myDir.x * playerDir.x + myDir.z * playerDir.z
    --dot = math.max(-1.0,math.min(1.0,dot)) -- 範囲を制限する
    if dot < -1.0 then
        dot = -1.0

    elseif  dot > 1.0 then
        dot = 1.0
    end

    local rate = ((dot + 1.0) / 2.0)

    local stopRate = 0.05
    if rate < stopRate  then
        rate = 0.0
    end


    -- 速度を求める
    local deltaTime = GetDeltaTime()
    finalSpeed =  speed * deltaTime * rate 

    -- プレイヤーへの方向を求める
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = targetTransform.translate.y - transform.translate.y
    dir.z = targetTransform.translate.z - transform.translate.z
    -- 正規化する
    dir:Normalize()

    -- 
    if rate > 0.0 then
    transform.translate.x = transform.translate.x + dir.x * finalSpeed
    transform.translate.y = transform.translate.y + dir.y * finalSpeed
    transform.translate.z = transform.translate.z + dir.z * finalSpeed
 
    else
    transform.translate.x = transform.translate.x - speed*0.075 

    end


    -- 回転
    -- if finalSpeed ~= 0.0 then
    -- local angle = math.atan(dir.x, dir.z)
    -- transform.rotate.y = angle
    -- myDir.x =  dir.x
    -- myDir.y =  dir.y
    -- myDir.z =  dir.z

    
    -- end

    local angle = math.atan(dir.x, dir.z)
    transform.rotate.y = angle
    myDir.x =  dir.x
    myDir.y =  dir.y
    myDir.z =  dir.z


    isHit = false
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function EaseOutQuint(st_, end_, t_)

    local convertedT = 1.0 - ( 1.0 - t_ ) ^ 5.0
    return Lerp(st_, end_, convertedT);

end

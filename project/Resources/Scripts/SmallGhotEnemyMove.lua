-- ロックオンするオブジェクトの名前
LockOnObjName = "objName"

local id = 0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

-- 弾のオブジェクト
bulletName = "EnemyBullet.json"

-- 弾を発射する時間(秒)
bulletShotTime = 1.0
-- 弾の発射を計測する
timer = 0.0

local actCnt = 0.0
local attackTime = 2.0
local idleTime = 2.0
local inv_attackTime = 0.0
local inv_idleTime = 0.0
local cur_action = 1
local default = 0.85

serchDistance = 3.0

aliveScriptName = "EnemyHp.lua"
isAliveName = "isAlive"


--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
inv_attackTime = 1.0 / attackTime
inv_idleTime = 1.0/ idleTime

end

--[[
    更新処理
--]]
function Update()
    
    -- アタッチ対象のid
    local thisId = this.GetEntityId()
    local isAlive = GetEntityScriptGlobal(thisId,aliveScriptName,isAliveName)

    local deltaTime = GetDeltaTime()

    timer = timer + (1.0 / (60.0 * bulletShotTime))
    
    id = GetEntity(LockOnObjName)
    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    targetTransform.translate.y = 0.0
    -- ベクトルを取得
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = 0.0
    dir.z = targetTransform.translate.z - transform.translate.z

    local distance = math.sqrt(dir.x*dir.x+dir.z*dir.z)



    -- 正規化する
    dir:Normalize()
    -- ターゲットの方向に向ける
    local angleY = math.atan(dir.x,dir.z)
    transform.rotate.y = angleY



    local defaultScale = 1.0
    if actCnt < 1.0 then

        if cur_action == 0 then
            local add_scale = 1.25
            local deltaSinCurve = math.sin(math.pi * actCnt)
            deltaSinCurve = EasingInBack(0.0, 1.0 , deltaSinCurve  )
            local result = deltaSinCurve *  add_scale

            actCnt = actCnt + inv_attackTime * deltaTime

            if actCnt > 1.0 then
                actCnt = 1.0
            end

            transform.rotate.x = EasingInBack(math.pi * 2.0, 0.0 , actCnt )
            local speed = EasingInBack(0.0,2.0,actCnt)* 2.0

            transform.scale.x = defaultScale + result  
            transform.scale.y = defaultScale + result  
            transform.scale.z = defaultScale + result  

            -- transform.translate.y = default
            transform:AddForward(speed * deltaTime)
            default = transform.translate.y 
            -- transform.translate.x = memorizeX
            -- transform.translate.z = memorizeZ



        else
            actCnt = actCnt + inv_idleTime * deltaTime
            local delta = math.sin(math.pi*actCnt)
            local conv1 = delta*0.5
            local conv2 = delta*0.125

            transform.translate.y = default + conv1 

            transform.scale.x = defaultScale+conv2 
            transform.scale.y = defaultScale+conv2 
            transform.scale.z = defaultScale+conv2 


        end

    else

        if cur_action == 0 then

            if isAlive == true then
                cur_action = 1
                local tmp = Transform.new()
                tmp.translate.x = transform.translate.x + dir.x * 0.05
                tmp.translate.z = transform.translate.z + dir.z * 0.05
                tmp.rotate.y = transform.rotate.y
                -- 弾を生成
                CreateEntity(bulletName,tmp)
 
            end


        else
            if distance<=serchDistance then
                cur_action = 0

            else
                cur_action = 1

            end

        end

        transform.scale.x = defaultScale
        transform.scale.y = defaultScale
        transform.scale.z = defaultScale
        -- transform.translate.y = default

        actCnt = 0.0
    end

    
end

function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
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
    local convertedT = 1.0 - tmp ^ 3.0

    return Lerp(st_, end_, convertedT)
end	

function EaseOutBack(st_, end_, t_)
    local c1 = 1.70158
    local c3 = c1 + 1.0

    local convertedT = 1.0 + c3 * (t_ - 1.0)*(t_ - 1.0)*(t_ - 1.0) + c1 * (t_ - 1.0) * (t_ - 1.0);

    return Lerp(st_, end_, convertedT)
end


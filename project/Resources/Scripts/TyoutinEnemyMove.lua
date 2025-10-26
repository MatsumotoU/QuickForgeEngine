-- プレイヤー
PlayerObjName = "ShotGunPlayer"

local id = 0

-- 速度
speed = 0.2
-- スロー速度
local slowSpeed = 0.0
-- 適応する速度
local moveSpeed = 0.0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

-- 移動距離
local moveDistance = 0.0

-- 弾の発射フラグ
local shotBullet = false

local moveNum = 3.0
local moveCnt = 0.0
moveTime = 2.0  
local inv_moveTime = 0.0

local idleCnt = 0.0
idleTime = 1.5
local inv_idleTime = 0.0


function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

--[[
    初期化処理
--]]
function Init()
id = GetEntity(PlayerObjName)
DebugLog("PlayerObjNameID: "..id)
-- スロー速度を求める
slowSpeed = speed * 0.5

inv_moveTime = 1.0/moveTime
inv_idleTime = 1.0/idleTime

end

--[[
    更新処理
--]]
function Update()

    local timer = GetDeltaTime()

    -- 角度
    transform.rotate.y = -1.6

    if isHit then
        moveSpeed = slowSpeed
    else
        moveSpeed = speed
    end

    -- 弾の発射を取得
    if QFE.Input.GetKeyTrigger("Shot") then
        if not shotBullet then
            shotBullet = true
            moveDistance = 0.0
        end
    end

    if shotBullet then
        -- 移動
        if moveCnt<1.0 then

            local jumpHeight = 1.5
            local defaultPosY = 0.0
            local max_slantAngle = math.pi * 0.25

            moveCnt = moveCnt + timer * inv_moveTime

            local loopNum =  math.sin(math.pi * moveCnt * moveNum)
            if loopNum < 0.0 then
                loopNum = loopNum * -1
            end


            local slantNum = max_slantAngle * loopNum
            transform.rotate.z = -max_slantAngle * loopNum 


            local jumpNum =loopNum  *jumpHeight
            transform.translate.y = defaultPosY + jumpNum 

            transform.translate.x = transform.translate.x - moveSpeed * timer

            if moveCnt >= 1.0 then
                shotBullet = false
                moveCnt = 0.0

            end

        end

    else
        local max_x = 22.5
        local max_z = 22.5

        idleCnt = idleCnt  + timer * inv_idleTime
        if idleCnt > 1.0  then
            idleCnt = 0.0
        end
        local cnv = EaseOutElastic(0.0,1.0,idleCnt)

        local angle = math.pi * 2.0* cnv
        transform.rotate.x = math.sin(angle )*timer*max_x
        transform.rotate.z = math.cos(angle )*timer*max_z

    end

end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end


function EaseOutElastic(st_, end_, t_)

    local c4 = (2.0 * 3.14159265359) / 3.0
    local convertedT = 2.0^(-10.0 * t_) * math.sin((t_ * 10.0 - 0.75) * c4) + 1.0

	return Lerp(st_, end_, convertedT);
end


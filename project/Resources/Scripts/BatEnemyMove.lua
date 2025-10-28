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

-- 移動フラグ
local isMove = false

-- 移動するタイプ(0:横, 1:縦)
local moveType = 0

-- 移動する方向
local dir = 1.0

-- マップ情報を取得するための名前
mapObjName = "MapGenerater"
generatorMapScriptName = "MapGenerater.lua"
varMapName = "map"
local linkID = 0
-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 2.0


local actCnt = 0.0
curveTime = 0.5
local inv_curveTime = 0.0
moveTime = 2.0
local inv_moveTime = 0.0
local cur_action = 0
local add_radNum = math.pi * 0.65
local delta_rad = add_radNum  
local cur_rad = -math.pi * 0.75 
local cur_rad2 = 6.7
local delta_rad2 = 6.7

--[[
    初期化処理
--]]
function Init()
    id = GetEntity(PlayerObjName)
    DebugLog("PlayerObjNameID: "..id)
    -- スロー速度を求める
    slowSpeed = speed * 0.5
    dir = 1.0
    inv_curveTime = 1.0 / curveTime
    inv_moveTime = 1.0 / moveTime

     -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    transform.rotate.z = cur_rad2
    transform.translate.y = 1.0

end

--[[
    更新処理
--]]
function Update()

    local deltaTime = GetDeltaTime()

    if actCnt <= 1.0 then

        if cur_action == 0 then
            local dst_Rad = cur_rad + delta_rad
            local dst_rad2 = cur_rad2 + delta_rad2

            actCnt = actCnt + inv_curveTime * deltaTime
            transform.rotate.y = EaseOutCubic(cur_rad, dst_Rad , actCnt )
            transform.rotate.z = EaseOutCubic(cur_rad2, dst_rad2 , actCnt )

        else
            actCnt = actCnt + inv_moveTime * deltaTime
            transform.rotate.x = math.sin(actCnt * math.pi * 12.0) * 0.4

        end

    else

        if cur_action == 0 then
            cur_action = 1
            cur_rad = cur_rad + delta_rad
            delta_rad = delta_rad * -1

            cur_rad2 = cur_rad2 + delta_rad2
            delta_rad2 = delta_rad2 * -1


        else
            cur_action = 0

        end

        actCnt = 0.0

    end

    transform:AddForward(speed * deltaTime)
    
    transform.translate.y = 1.0


end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end

    if obj.tag == "Wall" then
       isMove = false
    end
end


function Lerp(st_, end_, t_)

    return end_ * t_ + st_ * (1.0 - t_)
end

function EaseOutCubic(st_, end_, t_)
    local tmp = 1.0 - t_
    local convertedT = 1.0 - tmp ^ 3.0

    return Lerp(st_, end_, convertedT)
end	














-- -- プレイヤー
-- PlayerObjName = "ShotGunPlayer"

-- local id = 0

-- -- 速度
-- speed = 0.2
-- -- スロー速度
-- local slowSpeed = 0.0
-- -- 適応する速度
-- local moveSpeed = 0.0

-- -- 移動量
-- move = Vector3.new(0.0,0.0,0.0)

-- -- ヒットフラグ
-- local isHit = false

-- -- 移動フラグ
-- local isMove = false

-- -- 移動するタイプ(0:横, 1:縦)
-- local moveType = 0

-- -- 移動する方向
-- local dir = 1.0

-- -- マップ情報を取得するための名前
-- mapObjName = "MapGenerater"
-- generatorMapScriptName = "MapGenerater.lua"
-- varMapName = "map"
-- local linkID = 0
-- -- 取得したマップ
-- local map = {{},{}}
-- -- ブロックの大きさ
-- local kBlockSize = 2.0

-- --[[
--     初期化処理
-- --]]
-- function Init()
--     id = GetEntity(PlayerObjName)
--     DebugLog("PlayerObjNameID: "..id)
--     -- スロー速度を求める
--     slowSpeed = speed * 0.5
--     dir = 1.0

--      -- 生成したマップを取得
--     linkID = GetEntity(mapObjName)
--     DebugLog("LinkedID:"..linkID)
--     DebugLog("StartLoadMapData")
--     DebugLog(generatorMapScriptName)
--     DebugLog(varMapName)
--     map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
-- end

-- --[[
--     更新処理
-- --]]
-- function Update()

--     if isHit then
--         moveSpeed = slowSpeed
--     else
--         moveSpeed = speed
--     end

--     -- 弾の発射を取得
--     if QFE.Input.GetKeyTrigger("Shot") then
--         if not isMove then
--             isMove = true
--             moveType = math.random(0,1)

--             if moveType == 0 then
--                 transform.rotate.y = -1.6
--             end

--             -- 縦の場合は移動する方向を求める
--             if moveType == 1 then
--                local halfPosZ = GetHalfMapPosition()
--                if transform.translate.z >= halfPosZ then
--                     dir = -1.0
--                     transform.rotate.y = 3.2
--                else
--                     dir = 1.0
--                     transform.rotate.y = 0.0
--                end
--             end
--         end
--     end

--     if isMove then
--         local timer = GetDeltaTime()
--         if moveType == 0 then
--             transform.translate.x = transform.translate.x - moveSpeed * timer
--         else
--             transform.translate.z = transform.translate.z + (moveSpeed * timer) * dir
--         end
--     end
-- end

-- function OnCollisionEnter(id,obj)

--     if obj.tag == "SlowArea" then
--         isHit = true
--     end

--     if obj.tag == "Wall" then
--        isMove = false
--     end
-- end

-- -- 現在いるx位置のz軸の半分の位置を取得する
-- function GetHalfMapPosition()
--     local height = #map

--     -- マップ上での現在位置を取得する
--     local x = (transform.translate.x + kBlockSize) / kBlockSize

--     local isStartCount = false
--     local startZ = 0
--     local endZ = 0
--     for z = 1,height do
--         -- ステージの外周ブロックの始まりと終わりを取得する
--         if map[z][x] == 2 then
--             if isStartCount then
--                 endZ = z
--                 break
--             else
--                 startZ = z;
--                 isStartCount = true
--             end
--         end
--     end

--     local startPosZ =  (startZ - 1) * kBlockSize
--     local endPosZ = (endZ - 1) * kBlockSize

--     -- 半分の位置を取得
--     local halfPosZ = (startPosZ + endPosZ) / 2.0

--     return halfPosZ
-- end

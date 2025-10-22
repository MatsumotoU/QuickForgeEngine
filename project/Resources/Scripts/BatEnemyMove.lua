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
local kBlockSize = 1.0

--[[
    初期化処理
--]]
function Init()
    id = GetEntity(PlayerObjName)
    DebugLog("PlayerObjNameID: "..id)
    -- スロー速度を求める
    slowSpeed = speed * 0.5
    dir = 1.0

     -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
end

--[[
    更新処理
--]]
function Update()

    if isHit then
        moveSpeed = slowSpeed
    else
        moveSpeed = speed
    end

    -- 弾の発射を取得
    if QFE.Input.GetKeyTrigger("Shot") then
        if not isMove then
            isMove = true
            moveType = math.random(0,1)

            if moveType == 0 then
                transform.rotate.y = -1.6
            end

            -- 縦の場合は移動する方向を求める
            if moveType == 1 then
               local halfPosZ = GetHalfMapPosition()
               if transform.translate.z >= halfPosZ then
                    dir = -1.0
                    transform.rotate.y = 3.2
               else
                    dir = 1.0
                    transform.rotate.y = 0.0
               end
            end
        end
    end

    if isMove then
        local timer = GetDeltaTime()
        if moveType == 0 then
            transform.translate.x = transform.translate.x - moveSpeed * timer
        else
            transform.translate.z = transform.translate.z + (moveSpeed * timer) * dir
        end
    end
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end

    if obj.tag == "Wall" then
       isMove = false
    end
end

-- 現在いるx位置のz軸の半分の位置を取得する
function GetHalfMapPosition()
    local height = #map

    -- マップ上での現在位置を取得する
    local x = (transform.translate.x + 1.0) / 1.0

    local isStartCount = false
    local startZ = 0
    local endZ = 0
    for z = 1,height do
        -- ステージの外周ブロックの始まりと終わりを取得する
        if map[z][x] == 2 then
            if isStartCount then
                endZ = z
                break
            else
                startZ = z;
                isStartCount = true
            end
        end
    end

    local startPosZ =  (startZ - 1) * kBlockSize
    local endPosZ = (endZ - 1) * klockSize

    -- 半分の位置を取得
    local halfPosZ = (startPosZ + endPosZ) / 2.0

    return halfPosZ
end

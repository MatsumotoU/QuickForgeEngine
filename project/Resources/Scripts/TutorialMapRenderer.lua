
-- マップ情報を取得するための名前
mapObjName = "TutorialMapGenerater"
generatorMapScriptName = "TutorialMapGenerater.lua"
varMapName = "map"
local linkID = 0
-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 2.0
-- マップの横幅
local mapWidth = 0

-- 壁の名前
wallObjName = "Wall.json"

-- プレイヤーの位置を取得
playerName = "TutorialPlayer"
local playerID = 0

-- 描画する線
local linePosX = 0.0
local offsetX = 10.0

-- 移動した
local moveXIndex = 0

--[[
    初期化処理
--]]
function Init()
    -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    mapWidth = #map[1]

    -- 初期の見えているマップの範囲を描画する
    InitialzieDrawMap()

    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)
    local targetTransform = GetTransform(playerID)
    linePosX = targetTransform.translate.x
end

--[[
    更新処理
--]]
function Update()

    -- 見えている範囲をブロックを描画する
    UpdateDrawMap()
end

-- 初期の見えているマップの範囲を描画する
function InitialzieDrawMap()
    for z = 1,#map do
        for x = 1,20 do
            -- マップが空白の場合は飛ばす
            if map[z][x] ~= 0 then
                -- マップの位置を生成
                local xPos = (x - 1) * kBlockSize
                local zPos = (z - 1) * kBlockSize
                local tmpTransform = Transform.new()
                tmpTransform.translate.x = xPos
                tmpTransform.translate.y = 0.0
                tmpTransform.translate.z = zPos

                if map[z][x] == 1 then
                    CreateEntity(wallObjName,tmpTransform)
                elseif map[z][x] == 2 then
                    CreateEntity(wallObjName,tmpTransform)
                end
            end
        end
    end  
    moveXIndex = 20
end

function UpdateDrawMap()

    -- プレイヤーの位置を取得
    local targetTransform = GetTransform(playerID)

    -- 描画ラインを取得する
    if targetTransform.translate.x > linePosX then
        linePosX = targetTransform.translate.x
    end

    local xIndex = math.floor(((linePosX + offsetX) + kBlockSize) / kBlockSize)

    if moveXIndex < xIndex then
    
        if xIndex > mapWidth then
            return
        end

        for z = 1,#map do
            for x = moveXIndex + 1,xIndex do
                -- マップが空白の場合は飛ばす
                if map[z][x] ~= 0 then
                    -- マップの位置を生成
                    local xPos = (x - 1) * kBlockSize
                    local zPos = (z - 1) * kBlockSize
                    local tmpTransform = Transform.new()
                    tmpTransform.translate.x = xPos
                    tmpTransform.translate.y = 0.0
                    tmpTransform.translate.z = zPos

                    if map[z][x] == 1 then
                        CreateEntity(wallObjName,tmpTransform)
                    elseif map[z][x] == 2 then
                        CreateEntity(wallObjName,tmpTransform)
                    end
                end
            end
        end  
        -- 移動した位置にxを変更
        moveXIndex = xIndex
    end
end

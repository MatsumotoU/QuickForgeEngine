
-- 壁の名前
wallObjName = "name"
breakWallName = "breakName"

-- ブロックの大きさ
blockSize = 1.0

-- ステージの分割する数
mapDivision = 10

-- ステージの難易度
mapDifficulty = 8

-- マップ
map = {{},{}}

-- ステージ番号
stageNumber = 1

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

-- プレイヤーに関する名前
playerObjName = "ShotGunPlayer"
playerScriptName = "name.lua"
local playerID = 0

--[[
    初期化処理
--]]
function Init()
    stageNumber = 1
    DebugLog("mapInit")
    GenerateMap(mapDivision,mapDifficulty)
    --SpawnMap(map)

    -- 生成されたステージの難易度を表示
    local generatedMapDifficulty = EvaluateDifficulty(map, mapDivision)
    DebugLog(generatedMapDifficulty)

     -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
end

--[[
    更新処理
--]]
function Update()

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)

    if isReset then
        Reset()
    end
end

-- マップオブジェクトを出現させる
function SpawnMap(map)
    for z = 1,#map do
        for x = 1,#map[z] do
            -- マップが空白の場合は飛ばす
            if map[z][x] ~= 0 then
            
            -- マップの位置を生成
            local xPos = (x - 1) * blockSize
            local zPos = (z - 1) * blockSize
            local tmpTransform = Transform.new()
            tmpTransform.translate.x = xPos
            tmpTransform.translate.y = 0.0
            tmpTransform.translate.z = zPos

                if map[z][x] == 1 then
                    CreateEntity(breakWallName,tmpTransform)
                elseif map[z][x] == 2 then
                    CreateEntity(wallObjName,tmpTransform)
                end
            end
        end
    end
end

-- マップを生成する
function GenerateMap(division,targetDifficulty)

    if stageNumber == 1 then
        CreateMapOne()
    elseif stageNumber == 2 then
        CreateMapTwo()
    elseif stageNumber == 3 then
        CreateThree()
    else 
        CreateMapOne()
    end

    local width = #map[1]
    local height = #map

    DebugLog("EndBorderBlockCreated")

    -- 区画の幅を取得
    local divisionWidth = math.floor(width / division)

     -- ブロックが存在している区画の数
    local blockZoneCount = math.min(targetDifficulty,division)
    -- 空白の区画の数
    local emptyZoneCount = division - blockZoneCount

    -- ブロックを配置する区画の番号リストを取得する
    local blockZones = GetRandomBlockZones(division, blockZoneCount)

    DebugLog("GetBlockZones")

    for zone = 1, division do
        -- 現在の区画の範囲を取得する
        local startX = (zone - 1) * divisionWidth
        local endX = math.min(zone  * divisionWidth, width - 1)
        -- 最初の区画のみ外周分をなくす
        if zone == 1 then
            startX = startX + 2
            endX = endX - 2
        end

        -- ブロックを置く区画であるか判断する
        local hasBlock = Contains(blockZones,zone)

        -- ブロックを縦に2〜4マス、または2x2塊で配置
        if hasBlock then

            DebugLog("TryBlockSet")

            local placed = false
            local tryCount = 0
            local maxTry = 5 -- 配置を試みる最大回数

            while not placed and tryCount < maxTry do
                tryCount = tryCount + 1

                local pattern = math.random(1,2)

                DebugLog("pattern")
                DebugLog(pattern)

                if pattern == 1 then
                    -- ブロック縦に2〜4マスを取得
                    local sizeZ = math.random(2,4)
                    local size = {x = 1,z = sizeZ }
                    -- 配置可能な位置のデータを取得
                    local candidates = GetSetBlockPosition(map,startX,endX,size)
                    -- ブロックを配置
                    if #candidates > 0 then
                        local pos = candidates[math.random(1,#candidates)]
                        for dz = 0, size.z - 1 do
                            map[pos.z + dz][pos.x] = 1
                        end
                        placed = true
                    end
                else
                    -- 配置可能な位置のデータを取得
                    local size = {x = 2,z = 2}
                    -- 配置可能な位置のデータを取得
                    local candidates = GetSetBlockPosition(map,startX,endX,size)
                    -- 2x2ブロックを配置
                    if #candidates > 0 then
                        local pos = candidates[math.random(1,#candidates)]
                         for dz = 0,size.z - 1 do
                            for dx = 0,size.x - 1 do
                                map[pos.z + dz][pos.x + dx] = 1
                            end
                        end
                        placed = true
                    end
                end
            end

        end
    end

    DebugLog("EndMapGenerate")
end

-- リストに指定した値が存在してるかを取得する
function Contains(list, value)
    for i = 1, #list do
        if list[i] == value then
             return true
         end
    end
    return false
end

-- ブロックを配置する区画をランダムで取得する
function GetRandomBlockZones(total, selectCount)
    local indices = {}
    for i = 1, total do 
        indices[i] = i
    end

    -- 値をシャッフルする
    for i = #indices, 2, -1 do
        local j = math.random(1, i)
        indices[i], indices[j] = indices[j], indices[i]
    end

    local result = {}
    for i = 1, selectCount do
        result[i] = indices[i]
    end
    return result
end

-- 指定したサイズのブロックを配置出来るかを判断する
function CanSetBlock(x,size,map)
    -- ブロックの横幅を取得
    local sizeWidth = size.x - 1

     -- 指定した位置とその両端の空白を検索
    for stepX = x - 1, x + sizeWidth + 1 do
        -- 上下範囲を確認
        local isStartCount = false
        local heightBlank = 0
        for bz = 1,#map do
            -- 空白の数を計測する
            if isStartCount then
                heightBlank = heightBlank + 1
            end

            -- ステージの始まりと終わりを検出する
            if map[bz][stepX] == 2 then
                if isStartCount then
                    heightBlank = heightBlank - 1
                    break;
                else
                    isStartCount = true
                end
            end
        end

        -- ブロックを設置するための十分な空白がなければfalse
        if heightBlank <= 2 and size.z + 2 >= heightBlank then 
            return false
        end
    end

    -- 指定した位置とその両端に十分な空白があれば配置可能
    return true
end

-- 指定したサイズのブロックの配置可能な位置を取得
function GetSetBlockPosition(map,startX,endX,size)
    DebugLog("StartGetSetBlockPosition")
    -- 配置可能なx位置を取得
    local positionsX = {}
    --local height = #map
    for x = startX ,endX do
        if CanSetBlock(x,size,map) then
            table.insert(positionsX,{x = x})
        end
    end

    -- 配置可能な位置を登録する
     local positions = {}

     -- 配置可能なz位置を取得し、登録する
    for i = 1,#positionsX do
        local x = positionsX[i].x
        local isStartCount = false
        local startZ = 0
        local endZ = 0
        for bz = 1,#map do
            -- ステージの始まりと終わりを取得する
            if map[bz][x] == 2 then
                if isStartCount then
                    endZ = bz
                    break
                else
                    startZ = bz;
                    isStartCount = true
                end
            end
        end

        -- 配置可能なz位置を取得
        local positionZ = {}
        for z = startZ,endZ - size.z do
            for nextZ = z,z + size.z - 1 do
                if map[nextZ][x] == 2 then
                    break
                end
                -- 一番最後まで外周ブロックがなければ登録する
                if nextZ == z + size.z - 1 then
                    table.insert(positionZ,{z = z})
                end
            end
        end

        if #positionZ > 0 then
            local tmpX = positionsX[i].x
            local tmpZ = positionZ[math.random(1,#positionZ)].z
            -- 登録する
            table.insert(positions,{x = tmpX ,z = tmpZ})
        end
    end

    DebugLog("EndGetSetBlockPosition")

    return positions
end

-- 生成したマップの実際の難易度を測定する
function EvaluateDifficulty(map, divisionCount)
    local height = #map
    local width = #map[1]
    local divisionWidth = math.floor(width / divisionCount)
    local difficulty = 0

    for zone = 1, divisionCount do
        local startX = (zone - 1) * divisionWidth
        local endX = math.min(zone * divisionWidth, width - 1)

        -- 最初の区画のみ外周分をなくす
        if zone == 1 then
            startX = startX + 2
            endX = endX - 2
        end

        local hasBlock = false
        for z = 2, height - 1 do
            for x = startX, endX do
                if map[z][x] == 1 then
                    hasBlock = true
                    break
                end
            end
            if hasBlock then 
                break
             end
        end
        if not hasBlock then
            difficulty = difficulty + 1
        end
    end

    return difficulty
end

function CreateMapOne()

    local width = 100
    local height = 11

     -- 空のマップを作成する
    for z = 1, height do
        map[z] = {}
        for x = 1, width do
            map[z][x] = 0
        end
    end

    -- 外周の壁を埋める
    for z = 1, height do
        for x = 1, width do
            if x == 1 or z == 1 or x == width or z == height then
                map[z][x] = 2
            end
        end
    end
    
end

function CreateMapTwo()
    map = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
            {2,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}    }
end

function CreateThree()
    map = {{},{}}
end

-- リセット処理
function Reset()
    map = {}
    if stageNumber <= 2 then
        stageNumber = stageNumber + 1
    end
    GenerateMap(mapDivision,mapDifficulty)
end
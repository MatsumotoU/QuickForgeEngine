
-- プレイヤーの名前
playerName = "name"

-- 敵の名前
lockOnEnemyName = "name"
sliderEnemyName = "name"
floatyEnemyName = "name"

-- 敵の生存を管理する
local aliveScriptName = "EnemyCollsionBehavior.lua"
local isAliveVarName = "isAlive"

-- マップ情報を取得するための名前
mapObjName = "name"
generatorMapScriptName = "mapName"
varMapName = "varMapName"
local linkID = 0

-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 1.0

-- 敵を生成する範囲
local spawnRange = 10
-- 敵の最大数
local maxEnemyCount = 10
-- 敵の現在数
local currentEnemyCount = 0

-- 最大難易度
maxDifficulty = 5
-- 現在の難易度
local currentDifficulty = 0

-- 各敵の最大数
local maxEnemysCounts = {}
-- 各敵の現在数
local currentEnemysCounts = {}

-- 現在の生成されている敵のID
local enemiesIDList = {}

-- 内部用タイマー(仮)
local spawnTimer = 0
local spawnInterval = 3.0

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
    for z = 1,#map do
        DebugLog("Map:"..z)
       DebugLog(map[z][1])
       
    end

    -- 最大の敵の数を初期化
    table.insert(maxEnemysCounts,{name = lockOnEnemyName,maxCount = 2})
    table.insert(maxEnemysCounts,{name = sliderEnemyName,maxCount = 3})
    table.insert(maxEnemysCounts,{name = floatyEnemyName,maxCount = 3})
    -- 現在の敵の数を初期化
    table.insert(currentEnemysCounts,{name = lockOnEnemyName,difficulty = 2,count = 0})
    table.insert(currentEnemysCounts,{name = sliderEnemyName,difficulty = 1,count = 0})
    table.insert(currentEnemysCounts,{name = floatyEnemyName,difficulty = 1,count = 0})

    for i = 1,#currentEnemysCounts do
        DebugLog("LoadEnemyName :"..currentEnemysCounts[i].name)
    end

    DebugLog("EnemyGeneratorInit")
end

--[[
    更新処理
--]]
function Update()

    -- 敵の生成処理
    SpawnManager(3.0)

    -- 現在の敵の数を管理
    EnemyCountManager()
end

-- 敵を配置できる座標リストを取得
function GetAvailableEnemyPositions(cameraRightEdgeX)
    local positions = {}
    local width = #map[1]
    local height = #map
    local startX = math.max(1,math.floor(cameraRightEdgeX / kBlockSize))
    local endX = math.min(width - 2,math.floor((cameraRightEdgeX + spawnRange) / kBlockSize))

    for x = startX,endX do
        local isStartCount = false
        local startZ = 0
        local endZ = 0
        -- 配置出来るz位置の空間を取得
        for z = 1,height do
            -- ステージの始まりと終わりを取得する
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
        -- 配置可能な位置を登録
        for z = startZ,endZ do
            if map[z][x] == 0 then
                table.insert(positions,{x = x,z = z})
            end
        end
    end

    return positions
end

-- 生成する敵たちを取得
function CanSpawnEnemy()
    -- 現在の難易度が最大の難易度を超えていれば早期リターン
    if currentDifficulty > maxDifficulty then
        return 
    end

    -- 生成する敵のリスト
    local spawnEnemisList = {}

    -- 生成可能な難易度値を取得
    local tmpDifficulty = maxDifficulty - currentDifficulty
    -- 今回生成する難易度の総量を求める
    local targetDifficulty = math.random(1,tmpDifficulty)

    local sum = 0
    while sum < targetDifficulty do
        -- 敵をランダムで取得
        local index = math.random(1,#currentEnemysCounts)
        local candidate = currentEnemysCounts[index]

        -- 生成する敵の上限を求める
        local maxCount = 0
        for _,data in ipairs(maxEnemysCounts) do
            if data.name == candidate.name then
                maxCount = data.maxCount
            end
        end

        -- 生成する敵を登録する
        if candidate.count < maxCount and sum + candidate.difficulty <= targetDifficulty then
            table.insert(spawnEnemisList,{name = candidate.name})
            sum = sum + candidate.difficulty
            currentEnemysCounts[index].count = candidate.count + 1
            currentDifficulty = currentDifficulty + candidate.difficulty
        else
            break
        end
    end
    
    return spawnEnemisList
end

-- 敵を生成する
function SpawnEnemy(enemyName,Transform)
    -- 敵
    if currentEnemyCount > maxEnemyCount then
        return
    end

    -- 敵を生成する
    local id = CreateEntity(enemyName,Transform)

    -- 現在生成されている敵のidを登録
    table.insert(enemiesIDList,{id = id,name = enemyName})

    currentEnemyCount = currentEnemyCount + 1
end

-- 敵の生成を管理する
function SpawnManager(cameraRightEdgeX)

    spawnTimer = spawnTimer + (1.0 / (60.0 * 5))

    if spawnTimer >= 1.0 then
        spawnTimer = 0.0

        -- 配置出来る座標リストを追加
        local positions = GetAvailableEnemyPositions(cameraRightEdgeX)

        -- 配置する敵を求める
        local spawnEnemisList = CanSpawnEnemy()

        for i = 1,#spawnEnemisList do
            -- 生成する座標を取得する
            local index = math.random(1,#positions)
            local positionIndex = positions[index]
            local position = {x = positionIndex.x * 1.0,y = 0.0,z = positionIndex.z * 1.0}
            table.remove(positions,index)
            local tmpTransform = Transform.new()
            tmpTransform.translate.x = position.x
            tmpTransform.translate.y = 0.0
            tmpTransform.translate.z = position.z
            -- 敵を生成する
            DebugLog("SpawnEnemyName: ".. spawnEnemisList[i].name)
            SpawnEnemy(spawnEnemisList[i].name,tmpTransform)
        end
    end
end

-- 現在の敵の数を管理する
function EnemyCountManager()

    for i = 1,#enemiesIDList do

        local isAlive = GetEntityScriptGlobal(enemiesIDList[i].id,aliveScriptName,isAliveVarName)

        if not isAlive then
            for j = 1,#currentEnemysCounts do
                if enemiesIDList[i].name == currentEnemysCounts[j].name then
                    currentEnemysCounts[j].count = currentEnemysCounts[j].count - 1
                    currentEnemyCount = currentEnemyCount - 1
                    break
                end
            end
        end
    end
end
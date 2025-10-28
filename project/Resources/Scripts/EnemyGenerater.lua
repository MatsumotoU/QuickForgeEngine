
-- カメラの名前
cameraName = "name"
local targetId = 0
local targetTransform = Transform.new()

-- 敵幽霊の名前
normalGhostEnemyJson = "NormalGhostEnemy.json" -- 通常の幽霊
bigGhostEnemyJson = "BigGhostEnemy.json" -- 大きい幽霊
longGhostEnemyJson = "LongGhostEnemy.json" -- 長い幽霊
doubleGhostEnemyJson = "doubleGhostEnemy.json" -- 双子の幽霊
smallGhostEnemyJson = "SmallGhostEnemy.json" -- 小さい幽霊
boxGhostEnemyJson = "BoxGhostEnemy.json" -- 四角い幽霊
-- 有機物の敵の名前
tyoutinEnemyJson = "TyoutinEnemy.json" -- 提灯の敵
ratEnemyJson = "RatEnemy.json" -- ネズミの敵
zizouEnemyJson = "ZizouEnemy.json" -- 地蔵の敵
nasuEnemyJson = "NasuEnemy.json" -- ナスの敵
eyeEnemyJson = "EyeEnemy.json" -- 目玉の敵
batEnemyJson = "BatEnemy.json" -- コウモリの敵
pillBugEnemyJson = "PillBugEnemy.json" -- ダンゴムシの敵

-- 敵の生存を管理する
local aliveScriptName = "EnemyHp.lua"
local isAliveVarName = "isAlive"

-- マップ情報を取得するための名前
mapObjName = "MapGenerater"
generatorMapScriptName = "MapGenerater.lua"
varMapName = "map"
local linkID = 0

-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 2.0

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

-- 敵を生成するための変数
local lastCameraX = 0.0      -- 前回のプレイヤー位置
local movedDistance = 0.0    -- 移動距離を計算
local spawnDistance = 5.0    -- 敵を生成する距離
local offsetX = 15.0 -- カメラの画面端までの位置



-- 生成するステージの番号
stageNumber = 1


-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
varIsDeadName = "isDead"
local transitionID = 0

-- マップの番号に対応するテーブル
local enemyNumber = {{},{}}

-- 更新した時にマップデータをしっかりと読み込めるように辻褄を合わせる処理
local isLoadMap = false

--[[
    初期化処理
--]]
function Init()

    stageNumber = 2

    -- マップチップに対応する敵のテーブルを生成
    CreateEnemyNumberTable()

    -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)

    --ステージに対応する敵を登録する
    if stageNumber == 1 then
        StageOneRegisterEnemy()
    else
        StageTwoRegisterEnemy()
    end

    -- 追跡するidを取得
    targetId = GetEntity(cameraName)
    -- カメラの画面端までの位置
    offsetX = 15.0
    -- カメラの追跡する位置を設定
    local targetTransform = GetTransform(targetId)
    lastCameraX = targetTransform.translate.x + offsetX

    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    for z = 1,#map do
        DebugLog("Map:"..z)
       DebugLog(map[z][1])
    end

    for i = 1,#currentEnemysCounts do
        DebugLog("LoadEnemyName :"..currentEnemysCounts[i].name)
    end

    local positions = GetAvailableEnemyPositions(3.0)

    for i = 1,#positions do
        DebugLog("x:"..positions[i].x.."z:"..positions[i].z)
    end

    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)

    DebugLog("EnemyGeneratorInit")

    -- 正しく敵を生成出来るかを確認する(デバック用)-------------------------------------------------------------------
    local tmpTransform = Transform.new()
    tmpTransform.translate.y = 0.0
    tmpTransform.translate.x = 15.0
    tmpTransform.translate.z = 6.5
    CreateEntity(pillBugEnemyJson,tmpTransform)
    -- 正しく敵を生成出来るかを確認する(デバック用)-------------------------------------------------------------------

end

--[[
    更新処理
--]]
function Update()
    --DebugLog("currentDifficulty : ".. currentDifficulty)
    --DebugLog("currentEnemyCount : ".. currentEnemyCount)

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)
    -- リセット
    if isReset then
        Reset()
        if not isLoadMap then
            isLoadMap = true
        end
    end

    if not isReset then

        if isLoadMap then
            map = {{},{}}
            map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
            for z = 1,#map do
                DebugLog("Map:"..z)
                DebugLog(map[z][1])
            end
            isLoadMap = false
        end

    -- 敵の生成処理
    SpawnManager()

    -- 現在の敵の数を管理
    EnemyCountManager()
    end
end

-- 敵を配置できる座標リストを取得
function GetAvailableEnemyPositions(cameraRightEdgeX)
    local positions = {}
    local width = #map[1]
    local height = #map
    local startX = math.max(1,math.floor(cameraRightEdgeX / kBlockSize))
    local endX = math.min(width - 2,math.floor((cameraRightEdgeX + spawnRange) / kBlockSize))

    if endX <= startX then
        return positions
    end

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
        return {}
    end

    -- 生成する敵のリスト
    local spawnEnemisList = {}

    -- 生成可能な難易度値を取得
    local tmpDifficulty = maxDifficulty - currentDifficulty

    -- 生成可能な難易度が1以下なら早期リターン
    if tmpDifficulty <= 1 then
        return {}
    end

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
function SpawnManager()

    -- カメラの位置を取得
    targetTransform = GetTransform(targetId)
    local targetX = targetTransform.translate.x + offsetX

    -- 移動量を取得
    local delta = math.abs(targetX - lastCameraX)
    movedDistance = movedDistance + delta
    -- 前回の位置を更新
    lastCameraX = targetX

    -- 一定距離進んだら敵を生成
    if movedDistance >= spawnDistance then
        movedDistance = 0.0

        -- 配置出来る座標リストを追加
        local positions = GetAvailableEnemyPositions(targetX)

        -- 配置する敵を求める
        local spawnEnemisList = CanSpawnEnemy()

        -- 配置出来る敵がいなければ早期リターン
        if #spawnEnemisList <= 0 then
            return
        end

        if #positions <= 0 then
            return
        end

        for i = 1,#spawnEnemisList do
            -- 生成する座標を取得する
            local index = math.random(1,#positions)
            local positionIndex = positions[index]
            local position = {x = (positionIndex.x - 1) * kBlockSize,y = 0.0,z = (positionIndex.z - 1) * kBlockSize}
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

    -- 固定の敵を出す処理
    SpawnEnemiesFromMap(targetX)
end

-- 現在の敵の数を管理する
function EnemyCountManager()

    if #enemiesIDList < 1 then
        return
    end

    -- 削除する要素リスト
    local removeList = {}

    for i = 1,#enemiesIDList do

        local isAlive = GetEntityScriptGlobal(enemiesIDList[i].id,aliveScriptName,isAliveVarName)

        if not isAlive then
            DebugLog("isAlive : false")
            for j = 1,#currentEnemysCounts do
                if enemiesIDList[i].name == currentEnemysCounts[j].name then
                    -- 敵の数を減らす
                    currentEnemysCounts[j].count = currentEnemysCounts[j].count - 1
                    currentEnemyCount = currentEnemyCount - 1
                    -- 難易度を減らす
                    currentDifficulty = currentDifficulty - currentEnemysCounts[j].difficulty
                    if currentDifficulty < 0 then
                        currentDifficulty = 0
                    end
                    -- 削除リストに追加
                    table.insert(removeList,{index = i})
                    break
                end
            end
        end
    end

    -- リストから削除
    if #removeList > 0 then
        for i = 1, #removeList do
            table.remove(enemiesIDList,removeList[i].index)
        end
    end
end

-- ステージ1で使用する敵を登録
function StageOneRegisterEnemy()
    -- 最大の敵の数を初期化
    -- 幽霊の敵
    --table.insert(maxEnemysCounts,{name = normalGhostEnemyJson,maxCount = 2})
    table.insert(maxEnemysCounts,{name = longGhostEnemyJson,maxCount = 2})
    -- 有機物の敵
    table.insert(maxEnemysCounts,{name = tyoutinEnemyJson,maxCount = 1})
    table.insert(maxEnemysCounts,{name = ratEnemyJson,maxCount = 2})
    --四角いゴーストに変える
    table.insert(maxEnemysCounts,{name = zizouEnemyJson,maxCount = 2})
    
    -- 現在の敵の数を初期化
    -- 幽霊の敵
    --table.insert(currentEnemysCounts,{name = normalGhostEnemyJson,difficulty = 2,count = 0})
    table.insert(currentEnemysCounts,{name = longGhostEnemyJson,difficulty = 2,count = 0})
    -- 有機物の敵
    table.insert(currentEnemysCounts,{name = tyoutinEnemyJson,difficulty = 1,count = 0})
    table.insert(currentEnemysCounts,{name = ratEnemyJson,difficulty = 1,count = 0})
    --四角いゴーストに変える
    table.insert(currentEnemysCounts,{name = zizouEnemyJson,difficulty = 2,count = 0})
end

--ステージ2で使用する敵を登録
function StageTwoRegisterEnemy()
    -- 最大の敵の数を初期化
    -- 幽霊の敵
    table.insert(maxEnemysCounts,{name = normalGhostEnemyJson,maxCount = 1})
    table.insert(maxEnemysCounts,{name = bigGhostEnemyJson,maxCount = 2})
    table.insert(maxEnemysCounts,{name = doubleGhostEnemyJson,maxCount = 1})
    table.insert(maxEnemysCounts,{name = smallGhostEnemyJson,maxCount = 1})
    --箱幽霊の敵はいったんコメントアウトtable.insert(maxEnemysCounts,{name = boxGhostEnemyJson,maxCount = 1})
    -- 有機物の敵
    table.insert(maxEnemysCounts,{name = ratEnemyJson,maxCount = 3})
    table.insert(maxEnemysCounts,{name = batEnemyJson,maxCount = 2})
    table.insert(maxEnemysCounts,{name = pillBugEnemyJson,maxCount = 1})
    table.insert(maxEnemysCounts,{name = eyeEnemyJson,maxCount = 1})

    -- 現在の敵の数を初期化
    -- 幽霊の敵
    table.insert(currentEnemysCounts,{name = normalGhostEnemyJson,difficulty = 2,count = 0})
    table.insert(currentEnemysCounts,{name = bigGhostEnemyJson,difficulty = 1,count = 0})
    table.insert(currentEnemysCounts,{name = doubleGhostEnemyJson,difficulty = 3,count = 0})
    table.insert(currentEnemysCounts,{name = smallGhostEnemyJson,difficulty = 2,count = 0})
    --箱幽霊の敵はいったんコメントアウトtable.insert(currentEnemysCounts,{name = boxGhostEnemyJson,difficulty = 2,count = 0})
    -- 有機物の敵
    table.insert(currentEnemysCounts,{name = ratEnemyJson,difficulty = 1,count = 0})
    table.insert(currentEnemysCounts,{name = batEnemyJson,difficulty = 2,count = 0})
    table.insert(currentEnemysCounts,{name = pillBugEnemyJson,difficulty = 2,count = 0})
    table.insert(currentEnemysCounts,{name = eyeEnemyJson,difficulty = 2,count = 0})
end

-- リセット処理
function Reset()

    -- 現在いる敵のリストをリセット
    enemiesIDList = {}

    -- 生成する移動距離をリセット
    movedDistance = 0.0

    -- 敵の数と難易度をリセット
    currentEnemyCount = 0
    currentDifficulty = 0

    -- 敵のリストをリセット
    maxEnemysCounts = {}
    currentEnemysCounts = {}

    local isDead = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsDeadName)
    if isDead then
        -- stageNumber = 1
    else
        if stageNumber <= 2 then
            stageNumber = stageNumber + 1
        else
            stageNumber = 1
        end
    end

    if stageNumber == 1 then
        StageOneRegisterEnemy()
    else
        StageTwoRegisterEnemy()
    end

    -- 新しいマップの情報を取得する
    map = {{},{}}
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    --  for z = 1,#map do
    --     DebugLog("Map:"..z)
    --    DebugLog(map[z][1])
    -- end

    -- カメラ位置をリセット
    lastCameraX = 7.4
end

-- マップデータに基づいて敵を生成
function SpawnEnemiesFromMap(cameraRightEdgeX)
    local height = #map
    local width = #map[1]
    local startX = math.max(1,math.floor(cameraRightEdgeX / kBlockSize))
    local endX = math.min(width - 2,math.floor((cameraRightEdgeX + 2.0) / kBlockSize))

    if endX <= startX then
        return positions
    end

    for z = 1, height do
        for x = startX, endX do
            -- マップ番号を取得する
            local enemyType = map[z][x]
             if enemyType > 2 then
                DebugLog("EnemyFormMapType : "..enemyType)
                -- 番号に対応する敵を決定
               local spawnEnemyName = enemyNumber[enemyType].name
               -- 番号を空白に設定する
               map[z][x] = 0
                -- 敵を生成
                local tmp = Transform.new()
                tmp.translate.x = (x - 1) * kBlockSize
                tmp.translate.y = 0.0
                tmp.translate.z = (z - 1) * kBlockSize
                CreateEntity(spawnEnemyName, tmp)
            end
        end
    end
end

function CreateEnemyNumberTable()
    -- 番号に対応する敵のリストを作成する
    enemyNumber = {
       [1] = {name = "none"}, -- 1、障害物ブロック
       [2] = {name = "none"}, -- 2、番外壁ブロック
       [3] = {name = normalGhostEnemyJson}, -- 3、通常の幽霊
       [4] = {name = bigGhostEnemyJson},    -- 4、大きい幽霊
       [5] = {name = longGhostEnemyJson},   -- 5、長い幽霊
       [6] = {name = doubleGhostEnemyJson}, -- 6、双子の幽霊
       [7] = {name = smallGhostEnemyJson},  -- 7、小さい幽霊
       [8] = {name = tyoutinEnemyJson},     -- 8、ちょうちんの敵
       [9] = {name = ratEnemyJson},         -- 9、ネズミの敵
       [10] = {name = zizouEnemyJson},      -- 10、地蔵の敵
       [11] = {name = nasuEnemyJson},       -- 11、ナスの敵
       [12] = {name = eyeEnemyJson},        -- 12、目玉の敵
       [13] = {name = batEnemyJson},        -- 13、コウモリの敵
       [14] = {name = pillBugEnemyJson},    -- 14、ダンゴムシの敵
    }
end
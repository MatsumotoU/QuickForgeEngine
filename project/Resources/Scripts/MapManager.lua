
-- プレイヤーの位置を取得
playerName = "ShotGunPlayer"
local playerID = 0

-- マップ情報を取得するための名前
mapObjName = "MapGenerater"
generatorMapScriptName = "MapGenerater.lua"
varMapName = "map"
local linkID = 0
-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 1.0
-- マップの横幅
local mapWidth = 0

-- ゴールする位置
local goalPosX = 0.0

-- クリアしたかを判定するフラグ
isClear = false

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsRunName = "isRun"
local transitionID = 0

-- プレイヤーの死亡状態
isDead = false

-- HPを取得するための名前
hpScriptName = "HitPoint.lua"
varHpName = "hitPoint"

-- ステージ番号
local stageNumber = 1

function Init()
    -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    mapWidth = #map[1]
    goalPosX = (mapWidth - 2) * 1.0

    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)

    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)

    -- ステージ番号を取得
    stageNumber = 1
end

function Update()

    local isRun = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsRunName)

    if isRun then
        map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
        mapWidth = #map[1]
        goalPosX = (mapWidth - 2) * 1.0
    end

    if not isRun then
        isClear = false
        isDead = false
    end

    local targetTransform = GetTransform(playerID)

    -- プレイヤーが死亡しているか取得
    local hp = GetEntityScriptGlobal(playerID,hpScriptName,varHpName)
    if hp <= 0 then
        isDead = true
        isClear = true
        stageNumber = 1
        DebugLog("CurrentStageNumber :"..stageNumber)
    end

    -- マップのクリア判定を取得する
    if targetTransform.translate.x >= goalPosX then
        if not isClear then
        isClear = true
        stageNumber = stageNumber + 1
        DebugLog("CurrentStageNumber :"..stageNumber)
        end
    end

end

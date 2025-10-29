
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
local kBlockSize = 2.0
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

-- 操作するシーンのタイプ
sceneType = 0

-- ゲームオーバー時の選択タイプ
selectType = 0

-- ステージ番号
local stageNumber = 1

-- 有効になるまでの時間を求める
local timer = 0.0
local activeGameOverTime = 2.5
local activeClearTime = 2.0

local isGameOver = false

local stopTransform = Transform.new()

-- 音声
local selectSE = QFE.Audio.LoadSound("select.mp3")
local swichSE = QFE.Audio.LoadSound("uiMove.mp3")

function Init()
    -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
    mapWidth = #map[1]
    goalPosX = (mapWidth - 2) * kBlockSize

    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)

    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)

    -- ステージ番号を取得
    stageNumber = 1

    sceneType = 0
end

function Update()

    local isRun = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsRunName)

    if isRun then
        map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)
        mapWidth = #map[1]
        goalPosX = (mapWidth - 2) * kBlockSize
    else
        isClear = false
        isDead = false
        isGameOver = false

        if sceneType == 0 then
            NormalScene()
        elseif sceneType == 1 then
            GameOverScene()
        elseif sceneType == 2 then
            ClearScene()
        elseif sceneType == 3 then
            NextScene()
        end

    end
end

function NormalScene()
    local targetTransform = GetTransform(playerID)

    -- プレイヤーが死亡しているか取得
    local hp = GetEntityScriptGlobal(playerID,hpScriptName,varHpName)
    if hp <= 0 then
        if not isGameOver then
            sceneType = 1
            isGameOver = true
            CreateGameOverObj()
            timer = 0.0
            stopTransform.translate.x = targetTransform.translate.x
        end
    end

    -- マップのクリア判定を取得する
    if targetTransform.translate.x >= goalPosX then
        if not isClear then
        stageNumber = stageNumber + 1
        DebugLog("CurrentStageNumber :"..stageNumber)
            if stageNumber > 3 then
                sceneType = 2
                timer = 0.0
                CreateClearObj()
                stopTransform.translate.x = targetTransform.translate.x
            else
                CreateNextSceneObj()
                --isClear = true
                sceneType = 3
                timer = 0.0
                stopTransform.translate.x = targetTransform.translate.x
            end
        end
    end
end

-- ゲームオーバーシーンの処理
function GameOverScene()

    local targetTransform = GetTransform(playerID)
    targetTransform.translate.x = stopTransform.translate.x
    targetTransform.translate.z = 5.0

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= activeGameOverTime then
        -- リトライを選択
        if QFE.Input.GetKeyPress("MoveLeft") or QFE.Input.GetGamePadLeftStickDir().x < -0.5 then
            if selectType ~= 0 then
                QFE.Audio.PlaySound(swichSE,false,0.5)
            end
            selectType = 0
        end

        -- タイトルに戻るを選択
        if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetGamePadLeftStickDir().x > 0.5 then
            if selectType ~= 1 then
                QFE.Audio.PlaySound(swichSE,false,0.5)
            end
            selectType = 1
        end

        if QFE.Input.GetKeyTrigger("Shot") or  QFE.Input.GetGamePadTrigger(0x1000) then
            QFE.Audio.PlaySound(selectSE,false,0.5)

            if selectType == 0 then
                isDead = true
                isClear = true
                --stageNumber = 1
                sceneType = 0
                DebugLog("CurrentStageNumber :"..stageNumber)
            elseif selectType == 1 then
                -- タイトルシーンに移動
                --LoadScene("TitleScene")
                local tmpTrans = Transform.new()
                CreateEntity("FadeOutScene.json",tmpTrans)
            end
        end
    end
end

function NextScene()

    local targetTransform = GetTransform(playerID)
    targetTransform.translate.x = stopTransform.translate.x
    targetTransform.translate.z = 5.0

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= activeGameOverTime then
        -- リトライを選択
        if QFE.Input.GetKeyPress("MoveLeft") or QFE.Input.GetGamePadLeftStickDir().x < -0.5 then
            if selectType ~= 0 then
                QFE.Audio.PlaySound(swichSE,false,0.5)
            end
            selectType = 0

        end

        -- タイトルに戻るを選択
        if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetGamePadLeftStickDir().x > 0.5 then
            if selectType ~= 1 then
                QFE.Audio.PlaySound(swichSE,false,0.5)
            end
            selectType = 1
        end

        if QFE.Input.GetKeyTrigger("Shot") or QFE.Input.GetGamePadTrigger(0x1000) then
            QFE.Audio.PlaySound(selectSE,false,0.5)
            if selectType == 0 then
                isDead = false
                isClear = true
                --stageNumber = 1
                sceneType = 0
                DebugLog("CurrentStageNumber :"..stageNumber)
            elseif selectType == 1 then
                -- タイトルシーンに移動
                --LoadScene("TitleScene")
                local tmpTrans = Transform.new()
                CreateEntity("FadeOutScene.json",tmpTrans)
            end
        end
    end
end

-- クリアシーンの処理
function ClearScene()

    local targetTransform = GetTransform(playerID)
    targetTransform.translate.x = stopTransform.translate.x
    targetTransform.translate.z = 5.0

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= activeClearTime then
        -- タイトルに戻る
        if QFE.Input.GetKeyTrigger("Shot") or QFE.Input.GetGamePadTrigger(0x1000) then
            QFE.Audio.PlaySound(selectSE,false,0.5)
            -- タイトルシーンに移動
            --LoadScene("TitleScene")
            local tmpTrans = Transform.new()
            CreateEntity("FadeOutScene.json",tmpTrans)
        end
    end
end

-- ゲームオーバーシーンで使用するオブジェクト
function CreateGameOverObj()
    local tmpTransform = Transform.new()
    tmpTransform.translate.x = 1280.0
    CreateEntity("ResultSceneBg.json",tmpTransform)
    CreateEntity("RetryUI.json",tmpTransform)
    CreateEntity("SelectTitleUI.json",tmpTransform)
    CreateEntity("GameOverUI.json",tmpTransform)
    CreateEntity("arrowUI.json",tmpTransform)
end

-- 次のシーンで使用するオブジェクト
function CreateNextSceneObj()
     local tmpTransform = Transform.new()
    tmpTransform.translate.x = 1280.0
    CreateEntity("ResultSceneBg.json",tmpTransform)
    CreateEntity("NextSceneUI.json",tmpTransform)
    CreateEntity("SelectTitleUI.json",tmpTransform)
    CreateEntity("arrowUI.json",tmpTransform)
    CreateEntity("SecondClearUI.json",tmpTransform)

    tmpTransform.scale.x = 1.2
    tmpTransform.scale.y = 1.2
    CreateEntity("StageUI.json",tmpTransform)

    tmpTransform.scale.x = 1.5
    tmpTransform.scale.y = 1.5
    if stageNumber == 2 then
        CreateEntity("OneUI.json",tmpTransform)
    elseif stageNumber == 3 then
        CreateEntity("TwoUI.json",tmpTransform)
    end
end

-- クリアした時の使用するオブジェクト
function CreateClearObj()
    local tmpTransform = Transform.new()
    tmpTransform.translate.x = 1280.0
    CreateEntity("ResultSceneBg.json",tmpTransform)
    CreateEntity("ClearUI.json",tmpTransform)
    CreateEntity("ClearTitleUI.json",tmpTransform)
end
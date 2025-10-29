
-- プレイヤーの位置を取得
playerName = "TutorialPlayer"
local playerID = 0
bulletScriptName = "BulletShot.lua"
varIsReloadName = "isReload"
varIsShotName = "isShot"
varBullesName = "bullets"

-- 現在のイベントのタイプ
EventType = 1

-- イベント状態のリスト
local EventTypeList = {}

-- 現在イベント中の最終ラインを表示する位置
endLinePosX = 0.0

-- 移動量
local move = 0

-- 弾を打つ回数
local shotCount = 0
local reloadCount = 0
shotType = 0

-- 敵の名前
normalGhostEnemyJson = "TutorialGhost.json" -- 通常の幽霊
local normalGhostID = 0
ratEnemyJson = "TutorialEnemy.json" -- ネズミの敵
local ratEnemyID = 0
-- 敵の生存を管理する
local aliveScriptName = "TutorialEnemyHP.lua"
local isAliveVarName = "isAlive"

-- ゲージ管理
gaugeName = "Gauge"
local gaugeID = 0

-- 敵を倒した時のゲージ管理
gaugeTimer = 0.0

-- 出現させるUI
moveUIName = "TutorialMoveUI.json"
shotBulletUIName = "shotBulletUI.json"
reloadUIName = "reloadUI.json"
breakEnemyUI = "breakEnemyUI.json"
breakNormalEnemyUI = "breakNormalEnemyUI.json"
damegeBorderUIName = "damegeBorderUI.json"
damageExplaneUIName = "DamageExplanUI.json"
local isActive = false

NotGhostEnemyUIName = "NotGhostEnemyUI.json"
TombstoneExplanUIName = "TombstoneExplanUI.json"
tombstoneReloadUIName = "TombStoneReloadUI.json"

keyWASDUIName = "KeyWASDUI.json"
keyAUIName = "KeyAUI.json"
keySpaceUIName = "KeySpaceUI.json"

-- 最終ラインの描画用
endLineName = "EndLine"
local endLineID = 0

-- ダメージ床の名前
damageBorderName = "TutorialDamageBorder"
local damageBorderId = 0

local isHit = false

-- 音声
local clearSE = QFE.Audio.LoadSound("TutorialClearSE.mp3") 

function Init()
    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)
    -- ゲージを取得する
    gaugeID = GetEntity(gaugeName)
    local gaugeTransform = GetTransform(gaugeID)
    gaugeTransform.scale.x = 0.0
    -- イベントを登録する
    RegisterList()

    -- 移動を促すUIを生成
    local tmpTransform = Transform.new()
    tmpTransform.translate.x = 480.0
    tmpTransform.translate.y = 36.0
    CreateEntity(moveUIName,tmpTransform)
    tmpTransform.scale.x = 0.8
    tmpTransform.scale.z = 0.8
    tmpTransform.translate.x = 5
    tmpTransform.translate.y = -1.0
    tmpTransform.translate.z = 7.5
    CreateEntity(keyWASDUIName,tmpTransform)
    DebugLog("CreateMoveUI")
    -- 最終ラインの描画用
    endLineID = GetEntity(endLineName)

    -- ダメージ床を取得
    damageBorderId = GetEntity(damageBorderName)
    local borderTransform = GetTransform(damageBorderId)
end

function Update()
    -- イベントを管理する
    EventManager()
end

function EventManager()
    -- プレイヤーの位置を取得する
    local targetTransform = GetTransform(playerID)

    if not isHit then
    -- イベント中のラインを超えないようにする
    if targetTransform.translate.x > endLinePosX then
        targetTransform.translate.x = endLinePosX
    end
    end

    -- イベントタイプによって
    if EventType == 1 then
        EventOneScene()
    elseif EventType == 2 then
        EventTwoScene()
    elseif EventType == 3 then
        EventThreeScene()
    elseif EventType == 4 then
        EventFourScene()
    elseif EventType == 5 then
        EventFiveScene()
    end
end

function RegisterList()

    EventTypeList = {
       [1] = {endPosX = 10.0, isClear = false},
       [2] = {endPosX = 20.0, isClear = false},
       [3] = {endPosX = 30.0, isClear = false},
       [4] = {endPosX = 40.0, isClear = false},
       [5] = {endPosX = 50.0, isClear = false},
    }

    -- イベントライン
    endLinePosX = EventTypeList[1].endPosX
    local endLineTransform = GetTransform(endLineID)
    endLineTransform.translate.x = endLinePosX
end

-- シーン1
function EventOneScene()
    -- 移動量を取得する
    if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetKeyPress("MoveDown") or QFE.Input.GetKeyPress("MoveUp") or QFE.Input.GetKeyPress("MoveLeft") or QFE.Input.GetGamePadLeftStickDir():Length() > 0.3 then
        move = move + 1
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = move / 100.0
    end

    -- 一定量移動したら次のシーンに移行する
    if move >= 100 then
        -- 次のイベントを設定
        EventType = 2
        -- 最終ラインを取得
        endLinePosX = EventTypeList[EventType].endPosX
        local endLineTransform = GetTransform(endLineID)
        endLineTransform.translate.x = endLinePosX
        -- 弾を満タンにする
        local bullets = GetEntityScriptGlobal(playerID,bulletScriptName,varBullesName)
        bullets = 3
        -- ゲージをリセット
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = 0.0

        -- 弾の発射を促すUIを生成
        local tmpTransform = Transform.new()
        tmpTransform.translate.x = 400.0
        tmpTransform.translate.y = 36.0
        CreateEntity(shotBulletUIName,tmpTransform)
        tmpTransform.scale.x = 2.0
        tmpTransform.translate.x = 15.0
        tmpTransform.translate.y = -1.0
        tmpTransform.translate.z = 7.5
        CreateEntity(keySpaceUIName,tmpTransform)

        -- クリアの音を鳴らす
        QFE.Audio.PlaySound(clearSE,false,0.3)
    end
end

-- シーン2
function EventTwoScene()
    
    if shotType == 0 then
        -- 弾の発射を取得する
        local isShot = GetEntityScriptGlobal(playerID,bulletScriptName,varIsShotName)
        if isShot then
            shotCount = shotCount + 1
            local gaugeTransform = GetTransform(gaugeID)
            gaugeTransform.scale.x = shotCount / 3.0
        end

        if shotCount >= 3 then
            shotType = 1
            -- 弾を空にする
            local bullets = GetEntityScriptGlobal(playerID,bulletScriptName,varBullesName)
            bullets = 0
            -- ゲージをリセット
            local gaugeTransform = GetTransform(gaugeID)
            gaugeTransform.scale.x = 0.0

            -- リロードを促すUIを生成
            local tmpTransform = Transform.new()
            tmpTransform.translate.x = 412.0
            tmpTransform.translate.y = 36.0
            CreateEntity(reloadUIName,tmpTransform)
            tmpTransform.scale.x = 1.5
            tmpTransform.translate.x = 15.0
            tmpTransform.translate.y = -1.0
            tmpTransform.translate.z = 7.5
            CreateEntity(keyAUIName,tmpTransform)
        end

    elseif shotType == 1 then

        local bullets = GetEntityScriptGlobal(playerID,bulletScriptName,varBullesName)
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = bullets / 3.0

        if bullets >= 3 then
            -- 次のイベントを設定
            EventType = 3
            -- 最終ラインを取得
            endLinePosX = EventTypeList[EventType].endPosX
            local endLineTransform = GetTransform(endLineID)
            endLineTransform.translate.x = endLinePosX
            -- 幽霊の敵を生成
            local tmp = Transform.new()
            tmp.translate.x = 28.0
            tmp.translate.z = 4.0
            normalGhostID = CreateEntity(normalGhostEnemyJson, tmp)
            -- ゲージをリセット
            local gaugeTransform = GetTransform(gaugeID)
            gaugeTransform.scale.x = 0.0

            -- 敵を倒すことを促すUIを生成
            local tmpTransform = Transform.new()
            tmpTransform.translate.x = 480.0
            tmpTransform.translate.y = 36.0
            CreateEntity(breakEnemyUI,tmpTransform)

            -- クリアの音を鳴らす
            QFE.Audio.PlaySound(clearSE,false,0.3)
        end
    end

end

-- シーン3
function EventThreeScene()

    local isAlive = GetEntityScriptGlobal(normalGhostID,aliveScriptName,isAliveVarName)

    if not isAlive then
        -- ゲージ
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = 1.0
         -- 次のイベントを設定
        EventType = 4
        -- 最終ラインを取得
        endLinePosX = EventTypeList[EventType].endPosX
        local endLineTransform = GetTransform(endLineID)
        endLineTransform.translate.x = endLinePosX

         -- 幽霊の敵を生成
        local tmp = Transform.new()
        tmp.translate.x = 38.0
        tmp.translate.z = 4.0
        ratEnemyID = CreateEntity(ratEnemyJson, tmp)

        -- 敵を倒すことを促すUIを生成
        local tmpTransform = Transform.new()
        tmpTransform.translate.x = 480.0
        tmpTransform.translate.y = 36.0
        CreateEntity(breakNormalEnemyUI,tmpTransform)
        tmpTransform.translate.x = 32.0
        tmpTransform.translate.y = -1.0
        tmpTransform.translate.z = 7.5
        tmpTransform.scale.x = 2.0
        CreateEntity(NotGhostEnemyUIName,tmpTransform)

        -- クリアの音を鳴らす
        QFE.Audio.PlaySound(clearSE,false,0.3)
    end
end

-- シーン4
function EventFourScene()

    local deltatime = GetDeltaTime()
    gaugeTimer = gaugeTimer + deltatime

    if gaugeTimer >= 0.5 then
        -- ゲージをリセット
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = 0.0
    end

    local isAlive = GetEntityScriptGlobal(ratEnemyID,aliveScriptName,isAliveVarName)

    if not isAlive then
        gaugeTimer = 0.0
         -- 次のイベントを設定
        EventType = 5
        -- 最終ラインを取得
        endLinePosX = EventTypeList[EventType].endPosX
        local endLineTransform = GetTransform(endLineID)
        endLineTransform.translate.x = endLinePosX
        -- ゲージをリセット
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = 0.0

        -- ダメージ床の情報を促すUIを生成
        local tmpTransform = Transform.new()
        -- tmpTransform.translate.x = 323.0
        -- tmpTransform.translate.y = 36.0
        --CreateEntity(damegeBorderUIName,tmpTransform)
        tmpTransform.scale.x = 2.0
        tmpTransform.translate.x = 32.0
        tmpTransform.translate.y = -1.0
        tmpTransform.translate.z = 7.5
        CreateEntity(TombstoneExplanUIName,tmpTransform)

        -- tmpTransform.scale.x = 1.0
        -- tmpTransform.translate.x = 32.0
        -- tmpTransform.translate.y = -1.0
        -- tmpTransform.translate.z = 4.5
        --CreateEntity(tombstoneReloadUIName,tmpTransform)

        isActive = false

        -- クリアの音を鳴らす
        QFE.Audio.PlaySound(clearSE,false,0.3)
    end
end

-- シーン5
function EventFiveScene()
   local deltatime = GetDeltaTime()
    gaugeTimer = gaugeTimer + deltatime

    if gaugeTimer >= 6.0 then
        -- ゲージをリセット
        local gaugeTransform = GetTransform(gaugeID)
        gaugeTransform.scale.x = 0.0

        if not isActive then
            isActive = true
            local tmpTransform = Transform.new()
            tmpTransform.scale.x = 2.0
            tmpTransform.translate.x = 40.0
            tmpTransform.translate.y = -1.0
            tmpTransform.translate.z = 7.5
            CreateEntity(damageExplaneUIName,tmpTransform)

            tmpTransform.scale.x = 1.0
            tmpTransform.translate.x = 32.0
            tmpTransform.translate.y = 0.0
            tmpTransform.translate.z = 6.0
            CreateEntity(ratEnemyJson, tmpTransform)
            tmpTransform.translate.x = 32.0
            tmpTransform.translate.y = 0.0
            tmpTransform.translate.z = 3.0
            CreateEntity(ratEnemyJson, tmpTransform)
        end
    end

    local targetTransform = GetTransform(playerID)

    local borderTransform = GetTransform(damageBorderId)

    borderTransform.translate.x = borderTransform.translate.x + 5.0 * deltatime

    if targetTransform.translate.x <= borderTransform.translate.x then
        isHit = true
        DebugLog("isHit")
    end

    if borderTransform.translate.x >= endLinePosX + 5.0 then
        LoadScene("TitleScene")
    end
end

-- シーン6
function EventSixScene()
   
end

-- シーン7
function EventSevenScene()
    -- ゲームシーンに移動
    LoadScene("TitleScene")
end

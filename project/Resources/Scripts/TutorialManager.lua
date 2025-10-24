
-- プレイヤーの位置を取得
playerName = "TutorialPlayer"
local playerID = 0
bulletScriptName = "BulletShot.lua"
varIsReloadName = "isReload"
varIsShotName = "isShot"
varBullesName = "bullets"

-- 現在のイベントのタイプ
local EventType = 1

-- イベント状態のリスト
local EventTypeList = {}

-- 現在イベント中の最終ラインを表示する位置
endLinePosX = 0.0

-- 移動量
local move = 0

-- 弾を打つ回数
local shotCount = 0
local reloadCount = 0
local shotType = 0

-- 敵の名前
normalGhostEnemyJson = "NormalGhostEnemy.json" -- 通常の幽霊
local normalGhostID = 0
ratEnemyJson = "RatEnemy.json" -- ネズミの敵
local ratEnemyID = 0
-- 敵の生存を管理する
local aliveScriptName = "EnemyHp.lua"
local isAliveVarName = "isAlive"

function Init()
    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)
    -- イベントを登録する
    RegisterList()
end

function Update()
    -- イベントを管理する
    EventManager()
end

function EventManager()
    -- プレイヤーの位置を取得する
    local targetTransform = GetTransform(playerID)

    -- イベント中のラインを超えないようにする
    if targetTransform.translate.x > endLinePosX then
        targetTransform.translate.x = endLinePosX
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
       [5] = {endPosX = 40.0, isClear = false},
    }

    -- イベントライン
    endLinePosX = EventTypeList[1].endPosX
end

-- シーン1
function EventOneScene()
    -- 移動量を取得する
    if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetKeyPress("MoveDown") or QFE.Input.GetKeyPress("MoveUp") or QFE.Input.GetKeyPress("MoveLeft") then
        move = move + 1
    end

    -- 一定量移動したら次のシーンに移行する
    if move >= 100 then
        -- 次のイベントを設定
        EventType = 2
        -- 最終ラインを取得
        endLinePosX = EventTypeList[EventType].endPosX
        -- 弾を満タンにする
        local bullets = GetEntityScriptGlobal(playerID,bulletScriptName,varBullesName)
        bullets = 3
    end
end

-- シーン2
function EventTwoScene()
    
    if shotType == 0 then
        -- 弾の発射を取得する
        local isShot = GetEntityScriptGlobal(playerID,bulletScriptName,varIsShotName)
        if isShot then
            shotCount = shotCount + 1
        end

        if shotCount >= 3 then
            shotType = 1
            -- 弾を空にする
            local bullets = GetEntityScriptGlobal(playerID,bulletScriptName,varBullesName)
            bullets = 0
        end

    elseif shotType == 1 then
         local isReload = GetEntityScriptGlobal(playerID,bulletScriptName,varIsReloadName)
        -- リロードを取得する
        if isReload then
            reloadCount = reloadCount + 1
        end

        if reloadCount >= 3 then
            -- 次のイベントを設定
            EventType = 3
            -- 最終ラインを取得
            endLinePosX = EventTypeList[EventType].endPosX
            -- 幽霊の敵を生成
            local tmp = Transform.new()
            tmp.translate.x = 28.0
            tmp.translate.z = 4.0
            normalGhostID = CreateEntity(normalGhostaEnemyJson, tmp)
        end
    end

end

-- シーン3
function EventThreeScene()

    -- local isAlive = GetEntityScriptGlobal(normalGhostID,aliveScriptName,isAliveVarName)

    -- if not isAlive then
    --      -- 次のイベントを設定
    --     EventType = 4
    --     -- 最終ラインを取得
    --     endLinePosX = EventTypeList[EventType].endPosX
    -- end
end

-- シーン4
function EventFourScene()
    
end

-- シーン5
function EventFiveScene()
    
end

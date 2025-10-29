-- タイマー
local timer = 0.0

-- 入りの時間
InTime = 1.0

local startPosX = 1280.0
endPosX = 530.0

-- クリアされたかを取得する名前
mapManagerObjName = "MapManager"
mapManagerScriptName = "MapManager.lua"
varSelectTypeName = "selectType"
local linkID = 0

local isInScene = true

-- マテリアル
local mat = Material.new()

local waitTimer = 0.0
local maxWaitTime = 0.8

local transitionID = 0
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"

function Init()
    timer = 0.0
    -- 選択状態を取得する
    linkID = GetEntity(mapManagerObjName)
    isInScene = true
    -- マテリアル情報を取得
    mat = GetMaterial(GetThisEntityId())
     -- シーン遷移を取得
    transitionID = GetEntity("SceneTransitionManager")
end

function Update()

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)

    if isReset then
        destroy()
    end

    if isInScene then
        InScene()
    else
        SelectScene()
    end
    
end

function EaseIn(t)
    return t*t
end

function InScene()
    if timer <= 0.0 then
        transform.translate.y = 240.0
    end

    if timer <= InTime then
        local deltatime = GetDeltaTime()
        timer = timer + deltatime

        -- 0から1の範囲に収める
        local t = math.min(timer / InTime,1.0)

        transform.translate.x = QFE.Math.Leap(endPosX,startPosX,EaseIn(t))

        if timer >= InTime then
            transform.translate.x = endPosX
            isInScene = false
        end
    end
end

function SelectScene()

    local selectType = GetEntityScriptGlobal(linkID,mapManagerScriptName,varSelectTypeName)
    
    local delt = GetDeltaTime()
    waitTimer = waitTimer + delt

    -- 0から1の範囲に収める
    local ti = math.min(waitTimer / maxWaitTime,1.0)

    if ti <= 0.5 then
        mat.color.w = 1.0
    else
        mat.color.w = 0.0
    end

    if waitTimer >= maxWaitTime then
        waitTimer = 0.0
    end
end
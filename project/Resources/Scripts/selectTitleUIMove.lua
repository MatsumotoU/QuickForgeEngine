-- タイマー
local timer = 0.0

-- 入りの時間
InTime = 1.5

local startPosX = 1280.0
local endPosX = 750.0

-- クリアされたかを取得する名前
mapManagerObjName = "MapManager"
mapManagerScriptName = "MapManager.lua"
varSelectTypeName = "selectType"
local linkID = 0

local isInScene = true

-- マテリアル
local mat = Material.new()

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

function Init()
    timer = 0.0
    -- 選択状態を取得する
    linkID = GetEntity(mapManagerObjName)
    isInScene = true
    -- マテリアル情報を取得
    mat = GetMaterial(GetThisEntityId())
    mat.color.x = 0.2
    mat.color.y = 0.2
    mat.color.z = 0.2
     -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
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
        transform.translate.y = 420.0
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

    if selectType == 0 then
        mat.color.x = 0.2
        mat.color.y = 0.2
        mat.color.z = 0.2
    else
        mat.color.x = 1.0
        mat.color.y = 1.0
        mat.color.z = 1.0
    end
end

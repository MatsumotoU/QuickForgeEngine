
-- タイマー
local timer = 0.0

-- 入りの時間
InTime = 0.5

local startPosX = 1280.0
local endPosX = 0.0

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

function Init()
    timer = 0.0
     -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
end

function Update()

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)

    if isReset then
        destroy()
    end

    if timer <= 0.0 then
        transform.translate.y = 180.0
    end

    if timer <= InTime then
        local deltatime = GetDeltaTime()
        timer = timer + deltatime

        -- 0から1の範囲に収める
        local t = math.min(timer / InTime,1.0)

        transform.translate.x = QFE.Math.Leap(endPosX,startPosX,EaseIn(t))

        if timer >= InTime then
            transform.translate.x = endPosX
        end
    end
end

function EaseIn(t)
    return t*t
end

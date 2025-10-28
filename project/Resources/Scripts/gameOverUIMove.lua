-- タイマー
local timer = 0.0

-- 入りの時間
InTime = 1.0

local startPosX = 1280.0
local endPosX = 420.0

local waitTimer = 0.0
local maxWaitTime = 0.8

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

local mat = Material.new()

function Init()
    timer = 0.0
     -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
    mat = GetMaterial(GetThisEntityId())
end

function Update()

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)

    if isReset then
        destroy()
    end

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
        end

    else

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
end

function EaseIn(t)
    return t*t
end

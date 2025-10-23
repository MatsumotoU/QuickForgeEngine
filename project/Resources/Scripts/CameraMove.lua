targetObjName = "targetObjName"
offsetX = 1.0

local targetId = 0
local targetTransform = Transform.new()

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
varIsPreResetName = "isPreReset"
local transitionID = 0

--[[
    初期化処理
--]]
function Init()
    targetId = GetEntity(targetObjName)
    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)
end

--[[
    更新処理
--]]
function Update()

    -- 前状態をリセット
    local isPreReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsPreResetName)

    if isPreReset then
        PreReset()
    end

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)
    -- リセット
    if isReset then
        Reset()
    end

    if not isReset then
    targetTransform = GetTransform(targetId)

    if transform.translate.x + offsetX <= targetTransform.translate.x then
        transform.translate.x = targetTransform.translate.x - offsetX
    end
    end
    
end

-- リセットをする処理
function Reset()
    -- 位置をリセット
    transform.translate.x = 7.4
    local tx = transform.translate.x
    DebugLog("ResetCamera:"..tx)
end

function PreReset()
    -- 位置をリセット
    transform.translate.x = -100.0
    local tx = transform.translate.x
    DebugLog("PreResetCamera:"..tx)
end

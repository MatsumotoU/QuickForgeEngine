objName = "ShotGunPlayer"

borderX = 10.0
maxBorderX = 10.0
borderSpeed = 1.0
local id = 0
local targetX = 0.0

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

function Init()
    borderX = maxBorderX
    id = GetEntity(objName)
    targetX = 0.0

    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)
end

function Update()

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)
    -- リセット
    if isReset then
        Reset()
    end

    if not isReset then
    local deltaTime = GetDeltaTime()
    local nowPlayerposX = GetTransform(id).translate.x
    transform.rotate.z = transform.rotate.z + deltaTime * 3.0

    borderX = borderX - (borderSpeed * deltaTime)

    -- プレイヤーが離れすぎたら瞬時に追いつく
    if  nowPlayerposX - transform.translate.x > maxBorderX then
        borderX = maxBorderX  
        targetX = nowPlayerposX 
    end

    -- 位置更新
    transform.translate.x = targetX- borderX
    end
end

function Reset()
    transform.translate.x = -15.0
    targetX = 0.0

    DebugLog("DamageBorder : Reset")
end

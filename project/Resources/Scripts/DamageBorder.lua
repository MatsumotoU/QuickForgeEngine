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

-- 生成したゴーストのIDを保存
local ghostIdList = {}
local ghostMaxNum = 10
local maxMoveTime = 4.0

function Init()
    borderX = maxBorderX
    id = GetEntity(objName)
    targetX = 0.0

    -- ゴーストを生成する
    CreateGhost()

    transform.translate.y = -0.8

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
    -- transform.rotate.z = transform.rotate.z + deltaTime * 3.0

    borderX = borderX - (borderSpeed * deltaTime)

    -- プレイヤーが離れすぎたら瞬時に追いつく
    if  nowPlayerposX - transform.translate.x > maxBorderX then
        borderX = maxBorderX  
        targetX = nowPlayerposX 
    end

    -- 位置更新
    transform.translate.x = targetX- borderX
    end

    -- ゴーストの位置を更新
    UpdateGhost()

end

function Reset()
    transform.translate.x = -15.0
    targetX = 0.0

    UpdateGhost()

    DebugLog("DamageBorder : Reset")
end

-- ゴーストの生成処理
function CreateGhost()

    local tmp = Transform.new()
    tmp.translate.x = transform.translate.x
    tmp.translate.y = 0.5
    tmp.rotate.y = 1.6

    tmp.scale.x = 1.2
    tmp.scale.y = 1.2
    tmp.scale.z = 1.2

    for i = 1,ghostMaxNum do
        tmp.translate.z = (i-1) * 1.0 + 3.0
        local id = CreateEntity("SmallPlayerGhost.json",tmp)
        local timer = (i * 1) * 0.1
        table.insert(ghostIdList,{id = id,timer = timer})
    end

end

-- ゴースト側の更新処理
function UpdateGhost()

    local delta = GetDeltaTime()

    for i = 1,ghostMaxNum do
        local tmp = GetTransform(ghostIdList[i].id)
        tmp.translate.x = transform.translate.x + 0.2

        ghostIdList[i].timer = ghostIdList[i].timer + delta

        -- 0から1の範囲に収める
        local t = math.min(ghostIdList[i].timer / maxMoveTime,1.0)

        if t <= 0.5 then
            local inTimer = t / 0.5
            tmp.translate.y = QFE.Math.Leap(0.0,1.5,EaseOut(inTimer))
        else
            local outTimer = (t - 0.5) / 0.5
            tmp.translate.y = QFE.Math.Leap(1.5,0.0,EaseIn(outTimer))
        end

        if ghostIdList[i].timer >= maxMoveTime then
            ghostIdList[i].timer = 0.0
        end
    end
end

function EaseIn(t)
    return t * t
end

function EaseOut(t)
    return -t * (t - 2.0)
end

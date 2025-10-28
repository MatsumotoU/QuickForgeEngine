hitPoint = 10
max_hitPoint = 5
do_reload = false
korehaHidoi = false

damageTag = "enemy"
healTag = "item"


borderName = "DamageBorder"
local borderId = 0
local maxPosX = 0.0

maxDamageInterval = 1.0
local damageInterval = 0.0
local isDamaged = false
local frameCount = 0.0


-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

-- 音声
local damage = QFE.Audio.LoadSound("playerDamage.mp3")

-- リセットした時にノックバックを食らわないようにする
local waitTimer = 0.0
local maxWaitTime = 0.5

function Init()
    time = 0.0
    damageInterval = 0.0
    isDamaged = false
    borderId = GetEntity(borderName)

    -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
    pre_hitPoint = max_hitPoint
end

function Update()

    if waitTimer <= maxWaitTime then
        local deltatime = GetDeltaTime()
        waitTimer = waitTimer + deltatime
    end

    do_reload = false
    if korehaHidoi == true then
        do_reload = true
        korehaHidoi = false
    end

    -- リセット処理
    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)
    if isReset then
        Reset()
    end

    frameCount = frameCount + 1.0
    if hitPoint <= 0 then
        return
    end

    -- ボーダーダメージ
    if waitTimer >= maxWaitTime then
    if transform.translate.x < GetTransform(borderId).translate.x then
        --hitPoint = hitPoint - 1
        --do_reload = true
        --isDamaged = true
        force.velocity.x = force.velocity.x + 82.5
        --force.velocity.y = force.velocity.y + 20
        --damageInterval = maxDamageInterval
        CreateEntity("ExplotionParticleEmitter.json",transform)
    end
    end


    if damageInterval > 0.0 then
        damageInterval = damageInterval - 0.016
        transform.scale.y = math.sin(frameCount) * math.sin(frameCount)
        return
    else
        if isDamaged then
            isDamaged = false
            transform.scale.x = 1.0
            transform.scale.y = 1.0
            transform.scale.z = 1.0
        end
    end

    
end

function OnCollisionStay(id,obj)
    if damageInterval > 0.0 then
        return
    end

    if obj.tag == damageTag then

        if not GetEntityScriptGlobal(id,"EnemyHp.lua","isAlive") then
            return
        end
        damageInterval = maxDamageInterval  
        hitPoint = hitPoint - 1
        do_reload = true
        korehaHidoi = true
        isDamaged = true
        CreateEntity("ExplotionParticleEmitter.json",transform)
        QFE.Audio.PlaySound(damage,false,0.5)
    end
end

function Reset()
    hitPoint = max_hitPoint 
    pre_hitPoint = hitPoint  
    transform.translate.x = 1.7
    transform.translate.z = 6.0
    do_reload = false
    waitTimer = 0.0

    DebugLog("PlayerStateReset")
end
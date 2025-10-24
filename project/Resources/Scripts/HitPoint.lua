hitPoint = 10
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

function Init()
    time = 0.0
    damageInterval = 0.0
    isDamaged = false
    borderId = GetEntity(borderName)

    -- 生成したマップを取得
    transitionID = GetEntity(transitionObjName)
end

function Update()

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
    if transform.translate.x < GetTransform(borderId).translate.x then
        hitPoint = hitPoint - 1
        isDamaged = true
        force.velocity.x = force.velocity.x + 20
        force.velocity.y = force.velocity.y + 10
        damageInterval = maxDamageInterval
        CreateEntity("ExplotionParticleEmitter.json",transform)
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

    -- if maxPosX < transform.translate.x then
    --     maxPosX = transform.translate.x
    -- end    
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
        isDamaged = true
        CreateEntity("ExplotionParticleEmitter.json",transform)
    end
end

function Reset()
    hitPoint = 5
    transform.translate.x = 3.0
end
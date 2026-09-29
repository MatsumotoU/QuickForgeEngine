local breakSE = QFE.Audio.LoadSound("Down.wav")
local deathSE = QFE.Audio.LoadSound("zubashu.mp3")
hp = 1
local baseHp = hp

local scaleX = 0.0
local scaleY = 0.0
local scaleZ= 0.0

local hitInterval = 0.0
local damageMotionDuration = 0.4
local damageMotionElapsed = 0.0
local isEnemy = false
local enemyMaterial = nil
local damageMotionColor = Vector4.new(1.0, 1.0, 1.0, 1.0)
isDamageMotion = false
isBallDamageReaction = false
local damageOriginX = 0.0
local damageOriginZ = 0.0
local damageBaseRotationX = 0.0
local damageBaseRotationZ = 0.0
local damageDirectionX = 0.0
local damageDirectionZ = 0.0
local damageKnockbackDistance = 0.0
local damageTiltX = 0.0
local damageTiltZ = 0.0

local function ApplyDamageMotionColor(isWhite)
    if enemyMaterial == nil then
        return
    end

    if isWhite then
        enemyMaterial.color = Vector4.new(1.0, 1.0, 1.0, damageMotionColor.w)
    else
        enemyMaterial.color = damageMotionColor
    end
end

local function StartBallDamageReaction(ballId)
    local velocityX = GetEntityScriptGlobal(ballId, "Ball.lua", "collisionVelocityX") or 0.0
    local velocityZ = GetEntityScriptGlobal(ballId, "Ball.lua", "collisionVelocityZ") or 0.0
    local ballSpeed = math.sqrt(velocityX * velocityX + velocityZ * velocityZ)
    if ballSpeed <= 0.0001 then
        isBallDamageReaction = false
        return
    end

    isBallDamageReaction = true
    damageOriginX = transform.translate.x
    damageOriginZ = transform.translate.z
    damageBaseRotationX = transform.rotate.x
    damageBaseRotationZ = transform.rotate.z
    damageDirectionX = velocityX / ballSpeed
    damageDirectionZ = velocityZ / ballSpeed
    damageKnockbackDistance = math.min(ballSpeed * 0.05, 0.35)
    local tiltAngle = math.rad(15.0) * math.min(math.max(ballSpeed / 6.0, 0.5), 1.0)
    damageTiltX = damageDirectionZ * tiltAngle
    damageTiltZ = -damageDirectionX * tiltAngle
    transform.rotate.x = damageBaseRotationX + damageTiltX
    transform.rotate.z = damageBaseRotationZ + damageTiltZ
end

local function FinishBallDamageReaction()
    if not isBallDamageReaction then
        return
    end
    transform.translate.x = damageOriginX
    transform.translate.z = damageOriginZ
    transform.rotate.x = damageBaseRotationX
    transform.rotate.z = damageBaseRotationZ
    isBallDamageReaction = false
end

function Init()
    baseHp = hp
    scaleX = transform.scale.x
    scaleY = transform.scale.y
    scaleZ= transform.scale.z
    isEnemy = GetEntityTag(GetThisEntityId()) == "enemy"
    isDamageMotion = false
    isBallDamageReaction = false

    if isEnemy then
        enemyMaterial = GetMaterial(GetThisEntityId())
        if enemyMaterial ~= nil then
            local color = enemyMaterial.color
            damageMotionColor = Vector4.new(color.x, color.y, color.z, color.w)
        end
    end
end

function Update()
    local deltaTime = GetDeltaTime()

    if hitInterval > 0.0 then
        hitInterval = hitInterval - deltaTime
    end

    if isEnemy and isDamageMotion then
        damageMotionElapsed = damageMotionElapsed + deltaTime
        if damageMotionElapsed >= damageMotionDuration then
            isDamageMotion = false
            damageMotionElapsed = 0.0
            FinishBallDamageReaction()
            transform.scale.x = scaleX
            transform.scale.y = scaleY
            transform.scale.z = scaleZ
            ApplyDamageMotionColor(false)
        else
            local progress = damageMotionElapsed / damageMotionDuration
            local scalePulse = 1.0 + 0.2 * math.sin(progress * math.pi * 4.0)
            transform.scale.x = scaleX * scalePulse
            transform.scale.y = scaleY * scalePulse
            transform.scale.z = scaleZ * scalePulse

            if isBallDamageReaction then
                local outward = math.sin(progress * math.pi)
                local displacement = damageKnockbackDistance * outward * outward
                local rotationRecovery = (1.0 - progress) * (1.0 - progress)
                transform.translate.x = damageOriginX + damageDirectionX * displacement
                transform.translate.z = damageOriginZ + damageDirectionZ * displacement
                transform.rotate.x = damageBaseRotationX + damageTiltX * rotationRecovery
                transform.rotate.z = damageBaseRotationZ + damageTiltZ * rotationRecovery
            end

            local flashWhite = math.floor(damageMotionElapsed / 0.055) % 2 == 0
            ApplyDamageMotionColor(flashWhite)
            return
        end
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleZ,0.1)
end

function OnCollisionEnter(id,obj)
    if hitInterval > 0.0 then
        return
    end

    FinishBallDamageReaction()

    
    hp = hp-1
    if isEnemy then
        isDamageMotion = true
        damageMotionElapsed = 0.0
        if obj.tag == "ball" then
            StartBallDamageReaction(id)
        end
        ApplyDamageMotionColor(true)
    else
        transform.scale.x = scaleX * 1.3
        transform.scale.z = scaleZ * 1.3
    end
    local hitEmitterId = GetEntity("HitParticleEmitter")
    EmitParticles(hitEmitterId, transform.translate, 8, Vector3.new())
    if hp <= 0 then
        if isEnemy then
            StartHitStop()
            RunAllFunction("OnEnemyKilled")
        end
        local explosionEmitterId = GetEntity("ExplosionParticleEmitter")
        EmitParticles(explosionEmitterId, transform.translate, 32, Vector3.new())
        SetSceneGlobalData("Score",GetSceneGlobalData("Score") + 100 * baseHp)
        SetScore(GetScore() + 100 * baseHp)
        RunAllFunction("UpdateScore")
        local a = SimpleCreateEntity("SlashEffect.json")
        SetTranslate(a,transform.translate)
        QFE.Audio.PlaySound(deathSE,false,0.8)
        destroy()
    else
        QFE.Audio.PlaySound(breakSE,false,0.3)
        local a = SimpleCreateEntity("SmallSlash.json")
        SetTranslate(a,transform.translate)
    end
    hitInterval = 0.1
end

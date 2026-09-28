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

function Init()
    baseHp = hp
    scaleX = transform.scale.x
    scaleY = transform.scale.y
    scaleZ= transform.scale.z
    isEnemy = GetEntityTag(GetThisEntityId()) == "enemy"
    isDamageMotion = false

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

    
    hp = hp-1
    if isEnemy then
        isDamageMotion = true
        damageMotionElapsed = 0.0
        ApplyDamageMotionColor(true)
    else
        transform.scale.x = scaleX * 1.3
        transform.scale.z = scaleZ * 1.3
    end
    local hitEmitterId = GetEntity("HitParticleEmitter")
    EmitParticles(hitEmitterId, transform.translate, 8, Vector3.new())
    if hp <= 0 then
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

local moveSpeed = 4.5
local moveAcc = 0.7
local dashSpeed = 12.0
local damageInterval = 0.0
local invincibilityTimer = 0.0
local enemyKillInvincibilityDuration = 0.3
local dashBombRange = 3.0
local dashCooldownDuration = 5.0
local dashCooldownTimer = 0.0
canDash = true
local dashClearedEnemyBullets = {}

local moveTime = 0.0
local isStart = false
local scaleX = 0.0
local scaleY = 0.0
local isNearBall = true

local isMeshHeart = false

local strongBeatSE = QFE.Audio.LoadSound("StrongBeat.wav")
local damageSE = QFE.Audio.LoadSound("Bassdrum.wav")
local dashSE = QFE.Audio.LoadSound("Slash.wav")
local enemyBulletDestroySE = QFE.Audio.LoadSound("Down.wav")
local moveRotateY = 0.0

local beatId = 0
local startTime = 0.0

local function ClearEnemyBulletsAroundPlayer()
    local playerPosition = Vector3.new(
        transform.translate.x,
        transform.translate.y,
        transform.translate.z)
    local rangeSquared = dashBombRange * dashBombRange
    local explosionEmitterId = GetEntity("ExplosionParticleEmitter")

    for _, bulletId in ipairs(GetEntitiesByTag("enemyBullet")) do
        local bulletTransform = GetTransform(bulletId)
        if bulletTransform ~= nil then
            local bulletPosition = Vector3.new(
                bulletTransform.translate.x,
                bulletTransform.translate.y,
                bulletTransform.translate.z)
            local deltaX = bulletPosition.x - playerPosition.x
            local deltaY = bulletPosition.y - playerPosition.y
            local deltaZ = bulletPosition.z - playerPosition.z
            local distanceSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ
            if distanceSquared <= rangeSquared then
                dashClearedEnemyBullets[bulletId] = true
                if explosionEmitterId ~= -1 then
                    EmitParticles(explosionEmitterId, bulletPosition, 16, Vector3.new())
                end
                QFE.Audio.PlaySound(enemyBulletDestroySE, false, 0.3)
                DeleteEntityById(bulletId)
            end
        end
    end

    local bombEffectId = SimpleCreateEntity("BigHitCircleParticle.json")
    SetTranslate(bombEffectId, playerPosition)
    SetScale(bombEffectId, Vector3.new(dashBombRange, 1.0, dashBombRange))
    RunAllFunction("OnPlayerDash")
end

local function PerformDash(direction)
    force.velocity.x = dashSpeed * direction
    transform.scale.x = transform.scale.x * 1.25
    transform.scale.z = transform.scale.z * 0.8
    dashCooldownTimer = dashCooldownDuration
    canDash = false
    local dashDirection = Vector3.new(-direction, 0.0, 0.0)
    EmitParticles(GetEntity("DashParticleEmitter"), transform.translate, 12, dashDirection)
    QFE.Audio.PlaySound(dashSE, false, 0.5)
    ClearEnemyBulletsAroundPlayer()
end

function Init()
    scaleX = transform.scale.x
    scaleY = transform.scale.z
    beatId = GetEntity("Pacemaker")
end

function Update()
    dashClearedEnemyBullets = {}

    if dashCooldownTimer > 0.0 then
        dashCooldownTimer = math.max(0.0, dashCooldownTimer - GetDeltaTime())
    end
    canDash = dashCooldownTimer <= 0.0

    if invincibilityTimer > 0.0 then
        invincibilityTimer = math.max(0.0, invincibilityTimer - GetDeltaTime())
    end

    if GetEntityScriptGlobal(beatId,"Pacemaker.lua","bpm") <= 0 then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.rotate.y =transform.rotate.y+1.0
        return
    end

    if damageInterval > 0.0 then
        damageInterval = damageInterval - GetDeltaTime()
        transform.scale.x = math.abs(math.sin(damageInterval*10.0))
        transform.scale.z = math.abs(math.cos(damageInterval*10.0))
    end

    if QFE.Input.GetKeyTrigger("Jump") then
        startTime = 0.5
    end

    if startTime > 0.0 then
        if CountEntityTag("card") <= 0 then
            isStart =true
        end
        startTime = startTime - GetDeltaTime()
    end

    if isNearBall then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.3)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX*0.2,0.3)
    end
    aabbCollider.aabb.size.x = transform.scale.x*0.8

    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleY,0.1)
    transform.rotate.y = QFE.Math.SimpleEaseIn(transform.rotate.y+moveRotateY,3.14,0.1)

    if transform.scale.x <= 0.75 then
        if not isMeshHeart then
            ChangeMesh(GetThisEntityId(),"heart.obj")
            isMeshHeart = true
            local a = SimpleCreateEntity("BigHitCircleParticle.json")
            SetTranslate(a,transform.translate)
        end
    else
        if isMeshHeart then
            ChangeMesh(GetThisEntityId(),"Box1x1.obj")
            isMeshHeart = false
            local a = SimpleCreateEntity("BigHitCircleParticle.json")
            SetTranslate(a,transform.translate)
        end
    end

    if not isStart then
        return
    end
    -- ダッシュ
    if QFE.Input.GetKeyTrigger("Jump") and dashCooldownTimer <= 0.0 then
        if QFE.Input.GetKeyPress("MoveRight") then
            PerformDash(1.0)
        elseif QFE.Input.GetKeyPress("MoveLeft") then
            PerformDash(-1.0)
        end
    end

    -- 移動
    local isMove = false
    if QFE.Input.GetKeyPress("MoveRight") then
        if force.velocity.x < moveSpeed then
            force.velocity.x = force.velocity.x + moveAcc
        end
        moveTime = moveTime + 1.0
        moveRotateY = 0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveLeft") then
        if force.velocity.x > -moveSpeed then
            force.velocity.x = force.velocity.x - moveAcc
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveDown") then
        if force.velocity.z > -moveSpeed * 0.8 then
            force.velocity.z = force.velocity.z - moveAcc * 0.5
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveUp") then
        if force.velocity.z < moveSpeed * 0.8 then
            force.velocity.z = force.velocity.z + moveAcc * 0.5
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if not isMove then
        force.velocity.x = force.velocity.x *0.8
        force.velocity.z = force.velocity.z *0.8
        moveRotateY = 0.0
    end

    if moveTime > 20 then
        --Echo(transform.translate,0.8)
        moveTime = 0.0
    end

    if transform.translate.y ~= 0.0 then
        transform.translate.y = 0.0
    end


    if GetMinLengthToEntityFromTag("ball",transform.translate) <= 2.0 then
        isNearBall = true
    else
        isNearBall = false
    end
end

function OnCollisionEnter(id,obj)
    if obj.tag == "ball" then
        force.velocity.x = 0.0
        local x = (GetTransform(id).translate.x - transform.translate.x)
        transform.rotate.y = x * 10.0;
    end
end

function OnCollisionStay(id,obj)
    if obj.tag == "enemyBullet" or obj.tag == "Enemy" then
        if dashClearedEnemyBullets[id] then
            return
        end
        if damageInterval > 0.0 or invincibilityTimer > 0.0 then
            return
        end
        QFE.Audio.PlaySound(damageSE,false,0.8)

        local pacemakerId = GetEntity("Pacemaker")
        if pacemakerId ~= -1 then
            local currentBpm = GetEntityScriptGlobal(pacemakerId, "Pacemaker.lua", "bpm")
            if currentBpm then
                SetEntityScriptGlobal(pacemakerId, "Pacemaker.lua", "bpm", currentBpm - 10)
                DebugLog("Player Hit! BPM Reduced to: " .. tostring(currentBpm - 30))
                damageInterval = 2.5
            end
        end
        force.velocity.x = (transform.translate.x - GetTransform(id).translate.x)*5.0
        force.velocity.z = (transform.translate.z - GetTransform(id).translate.z)*5.0
		local hitDirection = Vector3.new(
			transform.translate.x - GetTransform(id).translate.x,
			0.0,
			transform.translate.z - GetTransform(id).translate.z)
		EmitParticles(GetEntity("HitParticleEmitter"), transform.translate, 16, hitDirection)

        local a = SimpleCreateEntity("BigHitCircleParticle.json")
        SetTranslate(a,transform.translate)

        RunAllFunction("OnPlayerDamage")
    end
end

function OnEnemyKilled()
    invincibilityTimer = enemyKillInvincibilityDuration
end

function OnStrongBeat()
    --transform.scale.x = scaleX * 1.2
    transform.scale.z = scaleY * 1.1
end

function OnBar()
    QFE.Audio.PlaySound(strongBeatSE,false,0.5)
    --transform.scale.x = scaleX * 1.1
    transform.scale.z = scaleY * 1.2
end

function OnNextStage()
    isStart = false
    transform.translate.x = 0.0
    transform.translate.z = -4.0
    force.velocity.x = 0.0
    force.velocity.z = 0.0
    force.acceleration.x = 0.0
    force.acceleration.z = 0.0
end

function OnStageClear()
    isStart = false
    startTime = 0.0
    force.velocity.x = 0.0
    force.velocity.y = 0.0
    force.velocity.z = 0.0
    force.acceleration.x = 0.0
    force.acceleration.y = 0.0
    force.acceleration.z = 0.0
end

function OnUpGradeSizeDownCard()
    DebugLog("PlayerSizeDown")
    scaleX = scaleX * 0.6
    scaleY = scaleY * 0.6
end

function OnUpGradeSpeedCard()
    DebugLog("SpeedUp")
    moveSpeed = moveSpeed + 1.5
end

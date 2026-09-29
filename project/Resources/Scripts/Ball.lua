isStart = false
dirX = 0.0
dirY = 0.0
speed = 2.0
collisionVelocityX = 0.0
collisionVelocityZ = 0.0

local baseSpeed = speed
local ballsId = {}

local timer =0.0
local isEnd =false

local wallHitSE = QFE.Audio.LoadSound("WallHit.wav")
local deathSE = QFE.Audio.LoadSound("damage.wav")
local BarSE = QFE.Audio.LoadSound("line.wav")
local collisionsLastFrame = {}
local collisionsThisFrame = {}
local enemyAimRange = 10.0
local enemyAimDotThreshold = math.cos(math.rad(15.0))

local function AimAtNearbyEnemy()
    local directionLength = math.sqrt(dirX * dirX + dirY * dirY)
    if directionLength <= 0.0001 then
        return
    end

    local bounceDirectionX = dirX / directionLength
    local bounceDirectionZ = dirY / directionLength
    local bestDot = enemyAimDotThreshold
    local bestDistance = math.huge
    local bestDirectionX = nil
    local bestDirectionZ = nil

    for _, enemyId in ipairs(GetEntitiesByTag("enemy")) do
        local enemyTransform = GetTransform(enemyId)
        if enemyTransform ~= nil then
            local toEnemyX = enemyTransform.translate.x - transform.translate.x
            local toEnemyZ = enemyTransform.translate.z - transform.translate.z
            local distanceSquared = toEnemyX * toEnemyX + toEnemyZ * toEnemyZ

            if distanceSquared > 0.0001 and distanceSquared <= enemyAimRange * enemyAimRange then
                local distance = math.sqrt(distanceSquared)
                local directionDot = (bounceDirectionX * toEnemyX + bounceDirectionZ * toEnemyZ) / distance
                if directionDot >= enemyAimDotThreshold and
                    (bestDirectionX == nil or directionDot > bestDot or
                        (math.abs(directionDot - bestDot) <= 0.000001 and distance < bestDistance)) then
                    bestDot = directionDot
                    bestDistance = distance
                    bestDirectionX = toEnemyX / distance
                    bestDirectionZ = toEnemyZ / distance
                end
            end
        end
    end

    if bestDirectionX ~= nil then
        dirX = bestDirectionX
        dirY = bestDirectionZ
    end
end

function Init()
    isStart = false
    dirX = 0.0
    dirY = 0.0
    collisionVelocityX = 0.0
    collisionVelocityZ = 0.0
    baseSpeed = speed
end

function Update()
    collisionsLastFrame = collisionsThisFrame
    collisionsThisFrame = {}

    local delta = GetDeltaTime()
    timer = timer + delta

    if speed > 6.0 then
        speed = 6.0
    end

    speed = QFE.Math.SimpleEaseIn(speed,baseSpeed,0.01)
    transform.rotate.y = transform.rotate.y + (speed *dirX* 0.05)

    -- 衝突判定より前の進行速度を保持する（反射後の dirX/dirY とは別）
    if isStart and not isEnd then
        collisionVelocityX = dirX * speed
        collisionVelocityZ = dirY * speed
    else
        collisionVelocityX = 0.0
        collisionVelocityZ = 0.0
    end

    if isEnd then
        --Echo(transform.translate,0.5)
        if timer > 3.0 then
            --RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            LoadScene("TitleScene.json")
        end
        return
    end
    
    if isStart then
        transform.translate.z = transform.translate.z + dirY * delta*speed
        transform.translate.x = transform.translate.x + dirX * delta*speed
    end

    if math.abs(dirX) + math.abs(dirY) > 2.0 then
        if timer > 0.3 then
            timer = 0.0
        end
    end

    if transform.translate.z <= -5.5 then
        --EchoForAudio(transform.translate,deathSE,0.5)
        destroy()
    end
end

function StartBall()
    if isStart or isEnd or CountEntityTag("card") > 0 then
        return
    end

    isStart = true
    dirX = 0.0
    dirY = 1.0
end

function OnCollisionEnter(id,obj)
    QFE.Audio.PlaySound(wallHitSE,false,0.2)
    local a = SimpleCreateEntity("HitCircleParticle.json")
    SetTranslate(a,transform.translate)
    EmitParticles(GetEntity("HitParticleEmitter"), transform.translate, 8, Vector3.new())
    --EchoForAudio(transform.translate,wallHitSE,0.2)
end

function OnCollisionStay(id,obj)
    local otherPosition = GetTransform(id).translate
    local deltaX = transform.translate.x - otherPosition.x
    local deltaZ = transform.translate.z - otherPosition.z
    local isNewCollision = collisionsLastFrame[id] ~= true and collisionsThisFrame[id] ~= true
    collisionsThisFrame[id] = true

    if obj.tag == "player" then
        if isNewCollision then
            dirX = deltaX
            if dirY < 0.0 then
                speed = speed + 3.5
            end
            if deltaZ < 0.0 then
                dirY = -math.abs(dirY)
            else
                dirY = math.abs(dirY)
            end
            AimAtNearbyEnemy()
        end
    elseif obj.tag == "sideWall" then
        if deltaX < 0.0 then
            dirX = -math.abs(dirX)
        elseif deltaX > 0.0 then
            dirX = math.abs(dirX)
        end
    elseif obj.tag == "topWall" then
        if deltaZ < 0.0 then
            dirY = -math.abs(dirY)
        elseif deltaZ > 0.0 then
            dirY = math.abs(dirY)
        end
    else
        if deltaX == 0.0 and deltaZ == 0.0 then
            if isNewCollision then
                if math.abs(dirX) >= math.abs(dirY) then
                    dirX = -dirX
                else
                    dirY = -dirY
                end
            end
            return
        end

        if math.abs(deltaX) > math.abs(deltaZ) then
            if deltaX < 0.0 then
                dirX = -math.abs(dirX)
            elseif deltaX > 0.0 then
                dirX = math.abs(dirX)
            end
        else
            if deltaZ < 0.0 then
                dirY = -math.abs(dirY)
            elseif deltaZ > 0.0 then
                dirY = math.abs(dirY)
            end
        end
    end
end

function OnNextStage()
    isStart = false
    isEnd = false
    collisionVelocityX = 0.0
    collisionVelocityZ = 0.0

    dirY = 0.0
    dirX = 0.0
end

function OnStrongBeat()
end


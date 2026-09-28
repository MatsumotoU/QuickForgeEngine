moveSpeed = 3.0
directionX = 0.0
directionZ = 1.0

local hasBounced = false
local isDestroyed = false
local hasTrailPosition = false
local lastTrailPosition = Vector3.new()
local trailEmitterId = -1
local hitEmitterId = -1

function Init()
    hasBounced = false
    isDestroyed = false
    hasTrailPosition = false
    trailEmitterId = GetEntity("EnemyBulletTrailEmitter")
    hitEmitterId = GetEntity("HitParticleEmitter")
end

function Update()
    local deltaTime = GetDeltaTime()
    transform.translate.x = transform.translate.x + directionX * moveSpeed * deltaTime
    transform.translate.z = transform.translate.z + directionZ * moveSpeed * deltaTime

    if trailEmitterId ~= -1 then
        local currentPosition = Vector3.new(
            transform.translate.x,
            transform.translate.y,
            transform.translate.z)
        if hasTrailPosition then
            local trailDirection = Vector3.new(
                lastTrailPosition.x - currentPosition.x,
                0.0,
                lastTrailPosition.z - currentPosition.z)
            if trailDirection:Length() > 0.0001 then
                EmitParticles(trailEmitterId, currentPosition, 1, trailDirection)
            end
        else
            hasTrailPosition = true
        end
        lastTrailPosition = currentPosition
    end
end

local function DestroyBullet()
    if isDestroyed then
        return
    end
    isDestroyed = true
    if hitEmitterId ~= -1 then
        EmitParticles(hitEmitterId, transform.translate, 8, Vector3.new())
    end
    delete()
end

function OnCollisionEnter(id, obj)
    if isDestroyed then
        return
    end

    local tag = obj.tag
    if tag == "enemy" or tag == "enemyBullet" then
        return
    end

    if tag == "sideWall" or tag == "topWall" then
        if hasBounced then
            DestroyBullet()
            return
        end

        hasBounced = true
        if tag == "sideWall" then
            directionX = -directionX
        else
            directionZ = -directionZ
        end
        local direction = Vector3.new(directionX, 0.0, directionZ)
        SetRotate(GetThisEntityId(), QFE.Math.LookAtFromDir(direction))
        return
    end

    DestroyBullet()
end

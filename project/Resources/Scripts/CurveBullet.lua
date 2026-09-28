moveSpeed = 10.0
turnSpeed = 0.3

local isHit = false
local hasTrailPosition = false
local lastTrailPosition = Vector3.new()
local trailEmitterId = -1
local hitEmitterId = -1

function Init()
    isHit = false
    hasTrailPosition = false
    trailEmitterId = GetEntity("EnemyBulletTrailEmitter")
    hitEmitterId = GetEntity("HitParticleEmitter")
end

function Update()
    local delta = GetDeltaTime()
    
    -- Rotate around Y axis (up)
    local turn = turnSpeed * delta
    transform.rotate.x = 0.0
    transform.rotate.y = transform.rotate.y + turn
    transform.rotate.z = 0.0
    
    transform:AddForward(moveSpeed * delta)

    local currentPosition = Vector3.new(
        transform.translate.x,
        transform.translate.y,
        transform.translate.z)
    if hasTrailPosition then
        local trailDirection = Vector3.new(
            lastTrailPosition.x - currentPosition.x,
            lastTrailPosition.y - currentPosition.y,
            lastTrailPosition.z - currentPosition.z)
        if trailDirection:Length() > 0.0001 then
            EmitParticles(trailEmitterId, currentPosition, 1, trailDirection)
        end
    else
        hasTrailPosition = true
    end
    lastTrailPosition = currentPosition
end

function OnCollisionEnter(id, obj)
    if isHit then
        return
    end
    
    local tag = GetEntityTag(id)
    if tag == "enemy" or tag == "enemyBullet" then
        return
    end

    isHit = true
    EmitParticles(hitEmitterId, transform.translate, 8, Vector3.new())
    delete()
end

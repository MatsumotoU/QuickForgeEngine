moveSpeed = 8.0
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
    
    -- Don't split if hitting the enemy that shot it? 
    -- Usually collision layer handles this, or tag check.
    -- Simple check: if hitting "enemy", ignore?
    -- `ClusterBullet` is likely on "enemyBullet" layer?
    -- LockOnEnemy is "enemy".
    -- Let's just split on anything for now, assuming proper layer setup.
    -- Or maybe better to filter out "enemy" tag to avoid instant explosion on launch?
    -- But usually bullets are spawned outside collision or have ignore rules.
    -- Let's stick to the prompt "hit something (何かに当たったら)".
    
    local tag = GetEntityTag(id)
    if tag == "enemy" or tag == "enemyBullet" then
        return
    end

    isHit = true
    EmitParticles(hitEmitterId, transform.translate, 12, Vector3.new())
    
    -- Spawn 8 bullets
    local myPos = transform.translate
    
    for i = 0, 7 do
        local angle = i * 45 -- 360 / 8 = 45
        local rad = math.rad(angle)
        
        -- Calculate direction in XZ plane (Y is up)
        local dir = Vector3.new(math.sin(rad), 0, math.cos(rad))
        
        local bulletID = SimpleCreateEntity("EnemyBullet.json")
        SetTranslate(bulletID, myPos)
        
        local rot = QFE.Math.LookAtFromDir(dir)
        SetRotate(bulletID, rot)
    end
    delete()
end

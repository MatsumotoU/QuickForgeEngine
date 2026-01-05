moveSpeed = 8.0
local isHit = false

function Init()
    isHit = false
end

function Update()
    local delta = GetDeltaTime()
    transform:AddForward(moveSpeed * delta)
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

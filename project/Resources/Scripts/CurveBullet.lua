moveSpeed = 10.0
turnSpeed = 0.3

local isHit = false

function Init()
    isHit = false
end

function Update()
    local delta = GetDeltaTime()
    
    -- Rotate around Y axis (up)
    local turn = turnSpeed * delta
    transform.rotate.x = 0.0
    transform.rotate.y = transform.rotate.y + turn
    transform.rotate.z = 0.0
    
    transform:AddForward(moveSpeed * delta)
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
    delete()
end

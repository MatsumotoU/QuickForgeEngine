moveSpeed = 10.0

local isHit = false

function Init()
    -- DebugLog("EnemyBullet Initialized")
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

    local tag = GetEntityTag(id)
    if tag == "player" then
        isHit = true
    end

    delete()
end

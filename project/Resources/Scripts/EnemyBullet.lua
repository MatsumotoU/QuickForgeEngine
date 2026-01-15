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

        
        local a = SimpleCreateEntity("SmallSlash.json")
        SetTranslate(a,transform.translate)
    end

    if obj.tag ~= "enemy" then
        delete()
    end
    
end

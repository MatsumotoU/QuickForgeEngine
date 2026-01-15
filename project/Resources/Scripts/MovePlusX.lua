moveSpeed = 2.0

function Init()
    DebugLog("MovePlusX.lua Initialized")
end

function Update()
    local delta = GetDeltaTime()
    transform.translate.x = transform.translate.x + (moveSpeed * delta)
end

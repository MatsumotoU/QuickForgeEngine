local timer = 0.0
vol = 1.0
speed = 1.0
local bossId = -1
local playerId = -1
local maxHealth = 0.0
local trackingSpeed = 1.0
local followDistance = 4.0

function Init()
    bossId = GetThisEntityId()
    playerId = GetEntity("PlayerBar")
    maxHealth = GetEntityScriptGlobal(bossId, "Block.lua", "hp") or 0.0
end

function Update()
    local deltaTime = GetDeltaTime()
    local currentHealth = GetEntityScriptGlobal(bossId, "Block.lua", "hp")

    if maxHealth <= 0.0 and currentHealth ~= nil then
        maxHealth = currentHealth
    end

    if maxHealth > 0.0 and currentHealth ~= nil and currentHealth <= maxHealth * 0.5 then
        if playerId == -1 then
            playerId = GetEntity("PlayerBar")
        end

        if playerId ~= -1 then
            local playerTransform = GetTransform(playerId)
            local deltaX = playerTransform.translate.x - transform.translate.x
            local deltaZ = playerTransform.translate.z - transform.translate.z
            local distance = math.sqrt(deltaX * deltaX + deltaZ * deltaZ)
            if distance > followDistance then
                local travelDistance = math.min(
                    trackingSpeed * deltaTime,
                    distance - followDistance)
                transform.translate.x = transform.translate.x + deltaX / distance * travelDistance
                transform.translate.z = transform.translate.z + deltaZ / distance * travelDistance
            end
            return
        end
    end

    timer = timer + deltaTime
    transform.translate.x = math.sin(timer*speed) * vol
end

shotInterval = 3.0
local timer = 0.0
local bossId = -1
local maxHealth = 0.0
local shotSE = QFE.Audio.LoadSound("byau.mp3")

local function IsDamageMotionActive()
    return GetEntityScriptGlobal(GetThisEntityId(), "Block.lua", "isDamageMotion") == true
end

local function SpawnBullet(direction)
    local bulletId = SimpleCreateEntity("BossBounceBullet.json")
    SetTranslate(bulletId, transform.translate)
    SetRotate(bulletId, QFE.Math.LookAtFromDir(direction))
    SetEntityScriptGlobal(bulletId, "BossBounceBullet.lua", "directionX", direction.x)
    SetEntityScriptGlobal(bulletId, "BossBounceBullet.lua", "directionZ", direction.z)
end

function Init()
    timer = 0.0
    bossId = GetThisEntityId()
    maxHealth = GetEntityScriptGlobal(bossId, "Block.lua", "hp") or 0.0
end

function Update()
    if CountEntityTag("StageStop") > 0 then
        return
    end

    local currentHealth = GetEntityScriptGlobal(bossId, "Block.lua", "hp")
    if maxHealth <= 0.0 and currentHealth ~= nil then
        maxHealth = currentHealth
    end
    if maxHealth <= 0.0 or currentHealth == nil or currentHealth > maxHealth * 0.5 then
        timer = 0.0
        return
    end

    if IsDamageMotionActive() then
        timer = 0.0
        return
    end

    timer = timer + GetDeltaTime()
    if timer >= shotInterval then
        timer = 0.0
        QFE.Audio.PlaySound(shotSE, false, 0.3)

        for i = 0, 7 do
            local angle = math.rad(i * 45.0)
            local direction = Vector3.new(math.sin(angle), 0.0, math.cos(angle))
            SpawnBullet(direction)
        end
    end
end

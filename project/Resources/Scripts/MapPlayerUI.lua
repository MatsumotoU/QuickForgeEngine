playerName = "ShotGunPlayer"
local playerID = 0

local time = 0.0

function Init()
    playerID = GetEntity(playerName)
end

function Update()
    time = time + GetDeltaTime()
    transform.scale.x = 1.0 + (math.sin(time) * 0.2)
    transform.scale.y = 1.0 + (math.sin(time) * 0.2)
    transform.scale.z = 1.0 + (math.sin(time) * 0.2)

    local tempTransform = GetTransform(playerID)
    local tempX = tempTransform.translate.x
    transform.translate.x = 97.0 + (1086.0 * (tempX / 140.0))
end

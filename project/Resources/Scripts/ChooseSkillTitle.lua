local time = 0.0
local baseX = 0.0
local baseY = 0.0
local baseRotationZ = 0.0

function Init()
    baseX = transform.translate.x
    baseY = transform.translate.y
    baseRotationZ = transform.rotate.z
end

function Update()
    time = time + GetDeltaTime()

    local sway = math.sin(time * 4.0)
    transform.translate.x = baseX + sway * 4.0
    transform.translate.y = baseY + math.sin(time * 4.0 + 1.0) * 5.0
    transform.rotate.z = baseRotationZ + sway * 0.02
end

local isAddForce = false

local timer = 0.0
local maxTime = 0.5

function Init()
    isAddForce = false
end

function Update()
    if not isAddForce then
        isAddForce = true
        local pitch = transform.rotate.x
        local yaw = transform.rotate.y

        local forward = Vector3.new()
        forward.x = math.cos(pitch) * math.sin(yaw)
        forward.y = math.sin(pitch)
        forward.z = math.cos(pitch) * math.cos(yaw)

        force.velocity.x = forward.x * 5.0
        force.velocity.y = 0.0
        force.velocity.z = forward.z * 5.0
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= maxTime then
        destroy()
    end
end
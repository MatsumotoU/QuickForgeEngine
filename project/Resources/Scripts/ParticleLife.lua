local isAddForce = false

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
        force.velocity.y = forward.y * 5.0
        force.velocity.z = forward.z * 5.0
    end

    if transform.translate.y <= -1.0 then
        destroy()
    end
end

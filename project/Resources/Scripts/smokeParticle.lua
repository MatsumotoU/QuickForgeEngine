local isAddForce = false
local timer = 0.0
local maxTime = 1.0

--[[
    初期化処理
--]]
function Init()
    isAddForce = false
end

--[[
    更新処理
--]]
function Update()
   if not isAddForce then
        isAddForce = true
        local pitch = transform.rotate.x
        local yaw = transform.rotate.y

        local forward = Vector3.new()
        forward.x = math.cos(pitch) * math.sin(yaw)
        forward.y = math.sin(pitch)
        forward.z = math.cos(pitch) * math.cos(yaw)

        transform.rotate.x = -0.9
        transform.rotate.y = 0.0
        transform.rotate.z = 0.0

        force.velocity.x = forward.x * 2.0
        force.velocity.y = 0.0
        force.velocity.z = forward.z * 2.0
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= maxTime then
        destroy()
    end
   
end
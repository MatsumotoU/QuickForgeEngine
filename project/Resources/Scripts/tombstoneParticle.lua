local isAddForce = false
-- バウンド時の減衰率
local bounceDamping = 0.9 

local boundCount = 0

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

        force.velocity.x = forward.x * 2.0
        force.velocity.y = forward.y * 4.0
        force.velocity.z = forward.z * 2.0
    end

    -- 地面に着いたらバウンドする処理
    if transform.translate.y <= 0.0 then
        force.velocity.y = -force.velocity.y * bounceDamping
        -- 3回バウンドしたら削除
        boundCount = boundCount + 1
        if boundCount > 3 then
            destroy()
        end
    end
end

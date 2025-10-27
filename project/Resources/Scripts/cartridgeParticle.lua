local isAddForce = false
-- バウンド時の減衰率
local bounceDamping = 0.9 

local boundCount = 0

-- 回転速度
local rotateSpeed = Vector3.new()

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
        local yaw = transform.rotate.y

        if transform.rotate.y == 0.0 then
            transform.translate.x = transform.translate.x + 0.8
        elseif transform.rotate.y >= 3.14 then
            transform.translate.x = transform.translate.x - 0.6
        end

        local forward = Vector3.new()
        forward.x = math.cos(0.0) * math.sin(yaw)
        forward.z = math.cos(0.0) * math.cos(yaw)

        rotateSpeed.x = math.random() * 4.0
        rotateSpeed.y = math.random() * 4.0
        rotateSpeed.z = math.random() * 4.0

        force.velocity.x = -forward.x * 2.0
        force.velocity.y = 2.0
        force.velocity.z = -forward.z * 2.0
    end

    -- 回転移動する
    local deltatime = GetDeltaTime()
    transform.rotate.x = transform.rotate.x + rotateSpeed.x * deltatime
    transform.rotate.y = transform.rotate.y + rotateSpeed.y * deltatime
    transform.rotate.z = transform.rotate.z + rotateSpeed.z * deltatime

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
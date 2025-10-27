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
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= maxTime then
        destroy()
    end
   
end

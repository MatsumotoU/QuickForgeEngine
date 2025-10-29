
local timer = 0.0
local max = 10.0

function Init()
timer = 0.0
end

function Update()
    
    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= max then
        destroy()
    end
end

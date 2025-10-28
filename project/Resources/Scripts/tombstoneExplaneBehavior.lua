local timer = 0.0
local maxTime = 6.0

function Init()

end

function Update()
    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= maxTime then
        destroy()
    end
end

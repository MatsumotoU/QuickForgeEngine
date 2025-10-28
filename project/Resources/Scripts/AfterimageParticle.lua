lifeTime = 0.3
local nowLife = lifeTime

function Init()
    nowLife = lifeTime
end

function Update()
    if nowLife > 0.0 then
        nowLife = nowLife - GetDeltaTime()
    else
        destroy()
    end
end

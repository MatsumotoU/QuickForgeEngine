maxLifeSpan = 1.0
lifeSpan = 0.0

function Init()
    lifeSpan = maxLifeSpan
end

function Update()
    if lifeSpan > 0.0 then
        lifeSpan = lifeSpan - 0.016
    else
        destroy()
    end
end

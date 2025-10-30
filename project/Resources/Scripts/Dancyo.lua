local time = 0.0
local frameCount = 0

function Init()

end

function Update()
    frameCount = frameCount + 1
    time = time + GetDeltaTime()
    transform.translate.y = 3.1 + math.sin(time) * 0.5

    if frameCount % 30 == 0 then
        local temp = Transform.new()
        temp.translate = GetTransform(GetEntity("DamageBorder")).translate
        temp.rotate.x = 1.0
        temp.scale.x = 0.5
        temp.scale.y = 0.5
        temp.scale.z = 0.5

        local rn = math.random(0,2)
        if rn == 0 then
            CreateEntity("SusumeParticle.json",temp)
        elseif rn == 1 then
            CreateEntity("OseParticle.json",temp)
        elseif rn == 2 then
            CreateEntity("HayoParticle.json",temp)
        end
    end
end

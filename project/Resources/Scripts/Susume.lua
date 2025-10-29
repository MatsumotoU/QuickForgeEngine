function Init()
    force.velocity.x = (math.random() + 1.0) * 3.0
    force.velocity.y = (math.random() + 1.0) * 8.0
    force.velocity.z = math.random() * 10.0
end

function Update()
    if transform.translate.y < -1.0 then
        destroy()
    end
end

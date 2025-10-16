function Init()

end

function Update()
    if transform.translate.y <= 0.0 then
        transform.translate.y = 0.0
        force.velocity.y = 0.0
        force.acceleration.y = 0.0
    end
end

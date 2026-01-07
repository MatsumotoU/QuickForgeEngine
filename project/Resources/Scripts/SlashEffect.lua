speed = 0.6

function Init()
    transform.rotate.y = math.random(0.0,10.0)
end

function Update()
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,0.0,speed)
    if transform.scale.z <= 0.1 then
        delete()
    end
end

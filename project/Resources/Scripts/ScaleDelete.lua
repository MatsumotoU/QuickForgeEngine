speed = 0.3

function Init()
end

function Update()
    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,speed)
    transform.scale.y= QFE.Math.SimpleEaseIn(transform.scale.y,0.0,speed)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,0.0,speed)

    if transform.scale:Length() <= 0.1 then
        delete()
    end
end

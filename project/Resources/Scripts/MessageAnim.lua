local animTime = 2.0

function Init()
    transform.scale.y = 0.0
end

function Update()
    if animTime > 0.0 then
        animTime = animTime - GetDeltaTime()
        transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.y,1.0,0.1)
        return
    end

    transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.y,0.0,0.1)
    if transform.scale.y <= 0.0 then
        destroy()
    end
end

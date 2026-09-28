holdTime = 2.5

function Init()

end

function Update()
    if holdTime > 0.0 then
        holdTime = holdTime - GetDeltaTime()
        transform.translate.y = QFE.Math.SimpleEaseIn(transform.translate.y,370.0,0.5)
    else
        transform.translate.y = QFE.Math.SimpleEaseIn(transform.translate.y,900.0,0.3)
        if transform.translate.y > 890.0 then
            delete()
        end
    end
end

local echoCharge = 0.0

function Init()
    echoCharge = 0.0
end

function Update()

    if echoCharge > 3.0 then
        --Echo(transform.translate,2.0)
        transform.rotate.x = 10.0
        echoCharge = 0.0
    else
        transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,0.0,0.1)
        echoCharge = echoCharge + GetDeltaTime()
    end
end

local targetRotateX = 0.0
local pumpActionTime = 0.0

function Init()
    targetRotateX = transform.rotate.x
end

function Update()
    if pumpActionTime > 0.0 then
        pumpActionTime = pumpActionTime - GetDeltaTime()
        transform.translate.y = QFE.Math.SimpleEaseIn(transform.translate.y,-0.5,0.5)
        transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,targetRotateX + 1.0,0.5)    
    end

    transform.translate.y = QFE.Math.SimpleEaseIn(transform.translate.y,0.0,0.3)
    transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,targetRotateX,0.3)
end

function Shot()
    transform.rotate.x = transform.rotate.x + 1.5
end

function Reroad()
    targetRotateX = targetRotateX + 6.28
end

function PumpAction()
    pumpActionTime = 0.3
end
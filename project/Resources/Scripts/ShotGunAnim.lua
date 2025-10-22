local targetRotateX = 0.0

function Init()
    targetRotateX = transform.rotate.x
end

function Update()
    transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,targetRotateX,0.3)
end

function Shot()
    transform.rotate.x = transform.rotate.x + 1.5
end

function Reroad()
    targetRotateX = targetRotateX + 6.28
end
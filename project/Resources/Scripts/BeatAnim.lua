local scaleX = 1.0
local scaleZ = 1.0

function Init()
    scaleX = transform.scale.x
    scaleZ = transform.scale.z
end

function Update()
    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleZ,0.1)
end

function OnStrongBeat()
    transform.scale.z = scaleZ * 1.1
end

function OnBar()
    transform.scale.x = scaleX * 1.1
    transform.scale.z = scaleZ * 1.2
end
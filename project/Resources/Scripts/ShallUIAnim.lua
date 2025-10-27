function Init()

end

function Update()
    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.5)
    transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.y,1.0,0.5)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,1.0,0.5)
end
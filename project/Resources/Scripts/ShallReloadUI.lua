function Init()

end

function Update()
    transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.y ,0.0,0.3)
end

function Anim()
    transform.scale.y = 1.5
end
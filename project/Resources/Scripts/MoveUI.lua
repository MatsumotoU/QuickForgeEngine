function Init()

end

function Update()
    if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetKeyPress("MoveLeft") then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.8,0.1)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.1)
    end

    if QFE.Input.GetKeyTrigger("MoveRight") or QFE.Input.GetKeyTrigger("MoveLeft") then
        transform.scale.x = transform.scale.x + 0.1
    end
end

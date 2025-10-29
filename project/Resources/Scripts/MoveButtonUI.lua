function Init()

end

function Update()
    local isPush = false
    if QFE.Input.GetKeyPress("MoveDown") then
        isPush = true
        transform.scale.x = transform.scale.x + 0.1
    end
    if QFE.Input.GetKeyPress("MoveLeft") then
        isPush = true
        transform.scale.x = transform.scale.x + 0.1
    end
    if QFE.Input.GetKeyPress("MoveRight") then
        isPush = true
        transform.scale.x = transform.scale.x + 0.1
    end
    if QFE.Input.GetKeyPress("MoveUp") then
        isPush = true
        transform.scale.x = transform.scale.x + 0.1
    end
    if QFE.Input.GetGamePadLeftStickDir():Length() > 0.5 then
        isPush = true
    end

    if isPush then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.8,0.5)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.5)
    end
end

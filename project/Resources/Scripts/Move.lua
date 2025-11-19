function Init()

end

function Update()
    if QFE.Input.GetKeyPress("MoveRight") then
        transform:AddRight(-0.1)
    end
    if QFE.Input.GetKeyPress("MoveLeft") then
        transform:AddRight(0.1)
    end
    if QFE.Input.GetKeyPress("MoveUp") then
        transform:AddForward(0.1)
    end
    if QFE.Input.GetKeyPress("MoveDown") then
        transform:AddForward(-0.1)
    end

    if transform.translate.y ~= 1.5 then
        transform.translate.y = 1.5
    end
end

local moveTime = 0.0
local isStart = false
function Init()

end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
        isStart =true
    end

    if not isStart then
        return 
    end

    if QFE.Input.GetKeyPress("MoveRight") then
        transform:AddRight(-0.1)
        moveTime = moveTime + 1.0
        
    elseif QFE.Input.GetKeyPress("MoveLeft") then
        transform:AddRight(0.1)
        moveTime = moveTime + 1.0
        
    end

    if moveTime > 20 then
        Echo(transform.translate,0.8)
        moveTime = 0.0
    end

    if transform.translate.y ~= 0.0 then
        transform.translate.y = 0.0
    end

    if transform.translate.z ~= -4.5 then
        transform.translate.z = -4.5
    end
end

mouseSpeed = 1.2

function Init()

end

function Update()
    local delatime = GetDeltaTime()
    transform.rotate.y = transform.rotate.y + (QFE.Input.GetMouseMoveDir().x * delatime) * mouseSpeed
    transform.rotate.x = transform.rotate.x + (QFE.Input.GetMouseMoveDir().y * delatime) * mouseSpeed
    if transform.rotate.x >= 0.9 then
        transform.rotate.x = 0.9
    end
    if transform.rotate.x <= -0.9 then
        transform.rotate.x = -0.9
    end

end

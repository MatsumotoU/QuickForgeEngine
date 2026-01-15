local timer = 0.0
local isPress = false

function Init()

end

function Update()
    local delta = GetDeltaTime()
    timer = timer + delta

    if QFE.Input.GetKeyTrigger("Jump") then
            isPress = true
    end

    if isPress then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.5)
        if transform.scale.x <= 0.0 then
            destroy()
        end
    else
        transform.scale.x = 1.0 + math.sin(timer) * 0.1
        transform.scale.y = 1.0 + math.sin(timer) * 0.1
    end

end

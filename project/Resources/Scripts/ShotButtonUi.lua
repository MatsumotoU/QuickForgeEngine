isPush = false
local time = 0.0

function Init()

end

function Update()
    if isPush then
        if time<1.0 then
            time = time + GetDeltaTime()
        else
            time = 0.0
            isPush = false
        end
    end

    if QFE.Input.GetKeyPress("Shot") then
        isPush = true
    end

    if isPush then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.8,0.5)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.5)
    end
end

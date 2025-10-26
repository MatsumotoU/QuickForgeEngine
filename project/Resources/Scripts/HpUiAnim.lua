local time = 0.0
local targetscaleX = 1.0
local targetscaleY = 1.0

function Init()
    targetscaleX = transform.scale.x
    targetscaleY = transform.scale.y
end

function Update()
    time = time + GetDeltaTime();
    if time > 1.0 then
        time = 0.0
        transform.scale.x = transform.scale.x + 0.5
        transform.scale.y = transform.scale.y + 0.5
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,targetscaleX,0.5)
    transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.y,targetscaleY,0.5)
end

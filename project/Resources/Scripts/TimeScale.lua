local timer = 0.0
speed = 1.0
vol = 0.1

function Init()

end

function Update()
    timer = timer + GetDeltaTime()
    transform.scale.x = 1.0 + math.sin(timer*speed) * vol
    transform.scale.y = 1.0 + math.sin(timer*speed) * vol
end
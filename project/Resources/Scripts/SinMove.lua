local timer = 0.0
vol = 1.0
speed = 1.0

function Init()

end

function Update()
    timer = timer + GetDeltaTime()
    transform.translate.x = math.sin(timer*speed) * vol
end

local time = 0.0

function Init()

end

function Update()
    time = time + GetDeltaTime()
    transform.rotate.z = math.sin(time) * 0.1
end

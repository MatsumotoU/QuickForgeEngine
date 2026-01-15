speed = 1.0

function Init()

end

function Update()
    local delta = GetDeltaTime()
    transform.rotate.z =transform.rotate.z+(speed * delta)
end

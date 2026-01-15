speed = 1.0
x = 0.0
y = 0.0
z = 0.0

function Init()

end

function Update()
    local delta = GetDeltaTime()
    transform.rotate.x =transform.rotate.x+(x * speed * delta)
    transform.rotate.y =transform.rotate.y+(y * speed * delta)
    transform.rotate.z =transform.rotate.z+(z * speed * delta)
end

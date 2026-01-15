Hp = 100

function Init()

end

function Update()
    transform.rotate.y = transform.rotate.y + GetDeltaTime()

    if GetDeltaTime() < 0.032 then
        SimpleCreateEntity("LuaCreateTest.json")
    end
end

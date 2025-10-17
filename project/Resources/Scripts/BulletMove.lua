moveSpeed = 1.0
aliveTime = 1.0

function Init()

end

function Update()
    local deltaTime = GetDeltaTime()

if aliveTime > 0.0 then
    aliveTime = aliveTime - deltaTime
else
    destroy()
end

    transform:AddForward(moveSpeed * deltaTime)
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" or obj.tag == "player" or obj.tag == "slowArea" then
        return
    end

    destroy()
end

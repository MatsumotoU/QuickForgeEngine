moveSpeed = 0.5
aliveTime = 0.75
maxSpeed = 20.0
minSpeed = 15.0 



function Init()
local randNum = math.random()

    moveSpeed = randNum * maxSpeed

    if moveSpeed <= minSpeed then
        moveSpeed = minSpeed + randNum * 1.25
    end

    transform.translate.z = transform.translate.z + randNum
end

function Update()
    local deltaTime = GetDeltaTime()
if aliveTime > 0.0 then
    aliveTime = aliveTime - deltaTime
else
    destroy()
end

    moveSpeed = moveSpeed * 0.95

    transform:AddForward(moveSpeed*deltaTime)
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" or obj.tag == "player" or obj.tag == "slowArea" then
        return
    end

    destroy()
end

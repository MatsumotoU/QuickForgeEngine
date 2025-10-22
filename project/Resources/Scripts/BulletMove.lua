moveSpeed = 0.5
aliveTime = 1.0
coe = 0.5
minSpeedCoe = 0.65 

function Init()

local randNum =math.random()

    if randNum <= minSpeed then
        randNum = minSpeed
    end

    moveSpeed = randNum * coe
end

function Update()
    local deltaTime = GetDeltaTime()
if aliveTime > 0.0 then
    aliveTime = aliveTime - deltaTime
else
    destroy()
end
    

    moveSpeed = moveSpeed * 0.95

    transform:AddForward(moveSpeed * deltaTime)
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" or obj.tag == "player" or obj.tag == "slowArea" then
        return
    end

    destroy()
end

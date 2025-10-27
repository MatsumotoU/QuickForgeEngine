moveSpeed = 0.5
aliveTime = 0.75
maxSpeed = 20.0
minSpeed = 15.0 

-- 音
local hit = QFE.Audio.LoadSound("hit.mp3")

function Init()
local randNum = math.random()

    moveSpeed = randNum * maxSpeed

    if moveSpeed <= minSpeed then
        moveSpeed = minSpeed + randNum * 1.25
    end

    transform.translate.z = transform.translate.z + randNum
end

function Update()

    transform.scale.x = 3.0
    transform.scale.y = 3.0
    transform.scale.z = 3.0

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

    if obj.tag == "enemy" or obj.tag == "Enemy" then
        QFE.Audio.PlaySound(hit,false,0.3)
    else
        QFE.Audio.PlaySound(hit,false,0.1)
    end
    
    destroy()
end

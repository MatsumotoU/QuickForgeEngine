moveSpeed = 0.5
aliveTime = 0.75
maxSpeed = 20.0
minSpeed = 15.0 

reduceCoe = 0.9

-- 音
local hit = QFE.Audio.LoadSound("hit.mp3")
local rotateNum = 0.0
local max_aliveTime = 0.0
local vanishStartRate = 0.0
local rate_beingWhite = 0.0

function Init()
local randNum = math.random()

    moveSpeed = randNum * maxSpeed

    if moveSpeed <= minSpeed then
        moveSpeed = minSpeed + math.random() * 1.25
    end

    transform.translate.z = transform.translate.z + randNum

   rotateNum = (math.random()+0.2) * math.pi * 5.0 * 0.0166666

   max_aliveTime = aliveTime
   vanishStartRate = 0.2 
   rate_beingWhite = 0.5
   
end

function Update()

    transform.translate.y = 0.5

    transform.scale.x = 2.5
    transform.scale.y = 2.5
    transform.scale.z = 2.5

    transform.rotate.z = transform.rotate.z +  rotateNum

    
    local deltaTime = GetDeltaTime()
if aliveTime > 0.0 then
    aliveTime = aliveTime - deltaTime

else
    destroy()
end

local cur_aliveRate = aliveTime / max_aliveTime 

if cur_aliveRate  <= rate_beingWhite then

    local mat = GetMaterial(GetThisEntityId()) 
    local rate_beWhite = 1.2

    mat.color.x = mat.color.x * rate_beWhite 
    mat.color.y = mat.color.y * rate_beWhite 
    mat.color.z = mat.color.z * rate_beWhite 

    if cur_aliveRate  <= vanishStartRate then
        local rate_vanish = 0.8
        mat.color.w = mat.color.w * rate_vanish 
    end

end


    moveSpeed = moveSpeed * reduceCoe

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

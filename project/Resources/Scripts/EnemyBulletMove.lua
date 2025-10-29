moveSpeed = 0.5
local aliveTime = 0.7
local maxSpeed = 20.0
minSpeed = 15.0 
local reduceCoe = 0.95


local rotateNum = 0.0
local max_aliveTime = 0.0
local vanishStartRate = 0.0
local rate_beingWhite = 0.0

function Init()

    moveSpeed = maxSpeed

     max_aliveTime = aliveTime
     vanishStartRate = 0.2 
     rate_beingWhite = 0.5
     rotateNum  = (math.random()+0.2) * math.pi * 5.0 * 0.0166666

end

function Update()

    transform.translate.y = 0.5

    transform.scale.x = 4.0
    transform.scale.y = 4.0
    transform.scale.z = 4.0

    transform.rotate.z = transform.rotate.z +  rotateNum

    local deltaTime = GetDeltaTime()
if aliveTime > 0.0 then
    aliveTime = aliveTime - deltaTime
else
    destroy()
end

    moveSpeed = moveSpeed * reduceCoe

    local cur_aliveRate = aliveTime / max_aliveTime 

if cur_aliveRate  <= rate_beingWhite then

    local mat = GetMaterial(GetThisEntityId()) 
    local rate_beWhite = 1.2

    -- mat.color.x = mat.color.x * rate_beWhite 
    mat.color.y = mat.color.y * rate_beWhite 
    mat.color.z = mat.color.z * rate_beWhite 

    if cur_aliveRate  <= vanishStartRate then

        local rate_vanish = 0.8
        mat.color.w = mat.color.w * rate_vanish 

    end

end

    transform:AddForward(moveSpeed*deltaTime)

end

function OnCollisionEnter(id,obj)

    if obj.tag == "Enemy" then

        return

    end

    destroy()
end

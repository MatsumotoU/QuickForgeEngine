hitPoint = 10
damageTag = "enemy"
healTag = "item"

scriptName = "script"
functionName = "func"

damageBorderX = 10.0
local maxPosX = 0.0

maxDamageInterval = 1.0
local damageInterval = 0.0
local isDamaged = false
local frameCount = 0.0

function Init()
    time = 0.0
    damageInterval = 0.0
    isDamaged = false
end

function Update()
    frameCount = frameCount + 1.0
    if hitPoint <= 0 then
        return
    end

    if damageInterval > 0.0 then
        damageInterval = damageInterval - 0.016
        transform.scale.y = math.sin(frameCount) * math.sin(frameCount)
        return
    else
        if isDamaged then
            isDamaged = false
            transform.scale.x = 1.0
            transform.scale.y = 1.0
            transform.scale.z = 1.0
        end
    end

    if maxPosX < transform.translate.x then
        maxPosX = transform.translate.x
    end

    if maxPosX - transform.translate.x > damageBorderX then
        hitPoint = hitPoint - 1
        force.velocity.x = force.velocity.x + 20
        force.velocity.y = force.velocity.y + 10
        damageInterval = maxDamageInterval
    end
end

function OnCollisionEnter(id,obj)
    if damageInterval > 0.0 then
        return
    end

    if obj.tag == damageTag then
        damageInterval = maxDamageInterval  
        hitPoint = hitPoint - 1
        isDamaged = true
        force.acceleration.x = force.acceleration.x - 30
        force.velocity.x = force.velocity.x - 10
    end
end
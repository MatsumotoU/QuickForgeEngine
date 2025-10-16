damageInterval = 0.0
maxDamageInterval = 0.5
hp = 5
maxHp = 5

local time = 0.0

stoneName = "hakaName"

function Init()
    hp = maxHp
    damageInterval = 0.0
    time = 0.0
end

function Update()
    time = time + 1.0

    if damageInterval > 0.0 then
        damageInterval = damageInterval - 0.016
        transform.scale.y = math.sin(time) * math.sin(time)
    else
        transform.scale.y = 1.0
    end
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" then
        hp = hp - 1
        damageInterval = maxDamageInterval

        if hp <= 0 then
            destroy()
            local temp = Transform.new()
            temp.translate.x = transform.translate.x
            temp.translate.y = transform.translate.y + 2.0
            temp.translate.z = transform.translate.z
            CreateEntity(stoneName,temp)
        end
    end
end
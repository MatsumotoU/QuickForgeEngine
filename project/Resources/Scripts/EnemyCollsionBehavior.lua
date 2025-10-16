
-- 生存フラグ
isAlive = true

function Init()
isAlive = true
end

function Update()

    if not isAlive then 
        destroy()
    end
end

function OnCollisionEnter(id,obj)

    if obj.tag == "Bullet" then
        isAlive = false
    end
end

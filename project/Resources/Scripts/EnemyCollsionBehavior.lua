
-- 生存フラグ
isAlive = true

function Init()
isAlive = true
end

function Update()

    if isAlive then 
        destroy()
    end
end

function OnCollisionEnter(id,obj)
    isAlive = false
end

moveSpeed = 1.0
aliveTime = 1.0

function Init()
    DebugLog("Init")
end

function Update()
if aliveTime > 0.0 then
    aliveTime = aliveTime - 0.016
else
    destroy()
end

    transform:AddForward(moveSpeed)
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" or obj.tag == "player" then
        return
    end

    destroy()
end

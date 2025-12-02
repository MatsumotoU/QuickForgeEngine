isStart = false
dirX = 0.0
dirY = 0.0
speed = 2.0

local timer =0.0
local isEnd =false

function Init()
    isStart = false
    dirX = 0.0
    dirY = 0.0
end

function Update()
    local delta = GetDeltaTime()
    timer = timer + delta

    if isEnd then
        Echo(transform.translate,0.5)
        if timer > 3.0 then
            RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            LoadScene("TitleScene.json")
        end
        return
    end

    
    if isStart then
        transform.translate.z = transform.translate.z + dirY * delta*speed
        transform.translate.x = transform.translate.x + dirX * delta*speed
    else
        if QFE.Input.GetKeyTrigger("Jump") then
            dirY = 1.0
            isStart = true;
        end
    end

    if math.abs(dirX) + math.abs(dirY) > 2.0 then
        if timer > 0.3 then
            timer = 0.0
            Echo(transform.translate,0.8)
        end
    end

    if transform.translate.z <= -5.5 then
        isEnd =true
        timer = 0.0
        Echo(transform.translate,10.0)
        Echo(transform.translate,5.0)
        Echo(transform.translate,3.0)
    end
end

function OnCollisionEnter(id,obj)
    Echo(transform.translate,1.0)
    if obj.tag == "player" then
        local x = -(GetTransform(id).translate.x - transform.translate.x)
        dirX = x
        dirY = math.abs(dirY)
    else
        local x = math.abs(GetTransform(id).translate.x - transform.translate.x) 
        local z = math.abs(GetTransform(id).translate.z - transform.translate.z) 
        if x > z then
            dirX =-dirX
        else
            dirY =-dirY
        end
    end

    
end
isStart = false
dirX = 0.0
dirY = 0.0
speed = 2.0

local ballsId = {}

local timer =0.0
local isEnd =false

local wallHitSE = QFE.Audio.LoadSound("WallHit.wav")
local deathSE = QFE.Audio.LoadSound("damage.wav")
local BarSE = QFE.Audio.LoadSound("line.wav")

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
            --RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            LoadScene("TitleScene.json")
        end
        return
    end

    
    if isStart then
        transform.translate.z = transform.translate.z + dirY * delta*speed
        transform.translate.x = transform.translate.x + dirX * delta*speed
    else
        if QFE.Input.GetKeyTrigger("Jump") then
            
            if CountEntityTag("card") <= 0 then
                isStart =true
                dirY = 1.0
            end
        end
    end

    if math.abs(dirX) + math.abs(dirY) > 2.0 then
        if timer > 0.3 then
            timer = 0.0
        end
    end

    if transform.translate.z <= -5.5 then
        --EchoForAudio(transform.translate,deathSE,0.5)
        destroy()
    end
end

function OnCollisionEnter(id,obj)
    --EchoForAudio(transform.translate,wallHitSE,0.2)
end

function OnCollisionStay(id,obj)
    if obj.tag == "player" then
        local x = -(GetTransform(id).translate.x - transform.translate.x)
        dirX = x
        dirY = math.abs(dirY)
    elseif obj.tag == "sideWall" then
        dirX =-dirX
    elseif obj.tag == "topWall" then
        dirY =-dirY
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

function OnNextStage()
    isStart = false
    isEnd = false

    dirY = 1.0
    dirX = 0.0
end

function OnStrongBeat()
end


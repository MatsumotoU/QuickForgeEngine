isStart = false
dirX = 0.0
dirY = 0.0
speed = 2.0

local baseSpeed = speed
local ballsId = {}

local timer =0.0
local isEnd =false

local wallHitSE = QFE.Audio.LoadSound("WallHit.wav")
local deathSE = QFE.Audio.LoadSound("damage.wav")
local BarSE = QFE.Audio.LoadSound("line.wav")
local collisionsLastFrame = {}
local collisionsThisFrame = {}

function Init()
    isStart = false
    dirX = 0.0
    dirY = 0.0
    baseSpeed = speed
end

function Update()
    collisionsLastFrame = collisionsThisFrame
    collisionsThisFrame = {}

    local delta = GetDeltaTime()
    timer = timer + delta

    if speed > 6.0 then
        speed = 6.0
    end

    speed = QFE.Math.SimpleEaseIn(speed,baseSpeed,0.01)
    transform.rotate.y = transform.rotate.y + (speed *dirX* 0.05)

    if isEnd then
        --Echo(transform.translate,0.5)
        if timer > 3.0 then
            --RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            LoadScene("TitleScene.json")
        end
        return
    end
    
    if isStart then
        transform.translate.z = transform.translate.z + dirY * delta*speed
        transform.translate.x = transform.translate.x + dirX * delta*speed
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

function StartBall()
    if isStart or isEnd or CountEntityTag("card") > 0 then
        return
    end

    isStart = true
    dirX = 0.0
    dirY = 1.0
end

function OnCollisionEnter(id,obj)
    QFE.Audio.PlaySound(wallHitSE,false,0.2)
    local a = SimpleCreateEntity("HitCircleParticle.json")
    SetTranslate(a,transform.translate)
    EmitParticles(GetEntity("HitParticleEmitter"), transform.translate, 8, Vector3.new())
    --EchoForAudio(transform.translate,wallHitSE,0.2)
end

function OnCollisionStay(id,obj)
    local otherPosition = GetTransform(id).translate
    local deltaX = transform.translate.x - otherPosition.x
    local deltaZ = transform.translate.z - otherPosition.z
    local isNewCollision = collisionsLastFrame[id] ~= true and collisionsThisFrame[id] ~= true
    collisionsThisFrame[id] = true

    if obj.tag == "player" then
        dirX = deltaX
        if dirY < 0.0 then
            speed = speed + 3.5
        end
        dirY = math.abs(dirY)
    elseif obj.tag == "sideWall" then
        if deltaX < 0.0 then
            dirX = -math.abs(dirX)
        elseif deltaX > 0.0 then
            dirX = math.abs(dirX)
        end
    elseif obj.tag == "topWall" then
        if deltaZ < 0.0 then
            dirY = -math.abs(dirY)
        elseif deltaZ > 0.0 then
            dirY = math.abs(dirY)
        end
    else
        if deltaX == 0.0 and deltaZ == 0.0 then
            if isNewCollision then
                if math.abs(dirX) >= math.abs(dirY) then
                    dirX = -dirX
                else
                    dirY = -dirY
                end
            end
            return
        end

        if math.abs(deltaX) > math.abs(deltaZ) then
            if deltaX < 0.0 then
                dirX = -math.abs(dirX)
            elseif deltaX > 0.0 then
                dirX = math.abs(dirX)
            end
        else
            if deltaZ < 0.0 then
                dirY = -math.abs(dirY)
            elseif deltaZ > 0.0 then
                dirY = math.abs(dirY)
            end
        end
    end
end

function OnNextStage()
    isStart = false
    isEnd = false

    dirY = 0.0
    dirX = 0.0
end

function OnStrongBeat()
end


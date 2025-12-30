local moveSpeed = 3.0
local moveAcc = 0.3
local dashSpeed = 8.0

local moveTime = 0.0
local isStart = false
local scaleX = 0.0
local scaleY = 0.0

local strongBeatSE = QFE.Audio.LoadSound("StrongBeat.wav")

function Init()
    scaleX = transform.scale.x
    scaleY = transform.scale.z
end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
        if CountEntityTag("card") <= 0 then
            isStart =true
        end
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleY,0.1)

    if not isStart then
        return 
    end
    -- ダッシュ
    if QFE.Input.GetKeyTrigger("Jump") then
        if QFE.Input.GetKeyPress("MoveRight") then
        force.velocity.x = dashSpeed
        
        elseif QFE.Input.GetKeyPress("MoveLeft") then
        force.velocity.x = -dashSpeed
        end
    end

    -- 移動
    if QFE.Input.GetKeyPress("MoveRight") then
        if force.velocity.x < moveSpeed then
            force.velocity.x = force.velocity.x + moveAcc
        end
        moveTime = moveTime + 1.0
        
    elseif QFE.Input.GetKeyPress("MoveLeft") then
        if force.velocity.x > -moveSpeed then
            force.velocity.x = force.velocity.x - moveAcc
        end
        moveTime = moveTime + 1.0
    else
        force.velocity.x = force.velocity.x *0.8
    end

    if moveTime > 20 then
        --Echo(transform.translate,0.8)
        moveTime = 0.0
    end

    if transform.translate.y ~= 0.0 then
        transform.translate.y = 0.0
    end

    if transform.translate.z ~= -4.5 then
        transform.translate.z = -4.5
    end

    
end

function OnCollisionEnter()
    force.velocity.x = 0.0
end

function OnStrongBeat()
    transform.scale.x = scaleX * 1.2
    transform.scale.z = scaleY * 1.1
end

function OnBar()
    EchoForAudio(transform.translate,strongBeatSE,0.5)
    transform.scale.x = scaleX * 1.1
    transform.scale.z = scaleY * 1.2
end

function OnNextStage()
    isStart = false
    transform.translate.x = 0.0
    force.velocity.x = 0.0
end
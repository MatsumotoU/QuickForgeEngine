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
        isStart =true
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleY,0.1)

    if not isStart then
        return 
    end

    if QFE.Input.GetKeyPress("MoveRight") then
        transform:AddRight(-0.1)
        moveTime = moveTime + 1.0
        
    elseif QFE.Input.GetKeyPress("MoveLeft") then
        transform:AddRight(0.1)
        moveTime = moveTime + 1.0
        
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

function OnStrongBeat()
    EchoForAudio(transform.translate,strongBeatSE,0.5)
    transform.scale.x = scaleX * 1.1
    transform.scale.z = scaleY * 1.1
end
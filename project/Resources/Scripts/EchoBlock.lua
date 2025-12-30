local echoCharge = 0.0
local currentBeat = 0
local echoSE = QFE.Audio.LoadSound("Slash.wav")

function Init()
    echoCharge = 0.0
end

function Update()
    transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,0.0,0.1)
end

function OnStrongBeat()
    transform.rotate.x = 5.0
end

function OnBar()
    EchoForAudio(transform.translate,echoSE,0.5)
    transform.rotate.x = 10.0
end

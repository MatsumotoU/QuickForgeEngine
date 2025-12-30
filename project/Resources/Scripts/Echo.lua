echoCharge = 0.0
echoMaxCharge = 2.5
canEcho = false
local breakSE = QFE.Audio.LoadSound("Slash.wav")

function Init()
    echoCharge = echoMaxCharge
    canEcho = true
end

function Update()
    echoCharge = echoCharge + GetDeltaTime()

    if QFE.Input.GetKeyTrigger("Jump") then
        if echoCharge >= echoMaxCharge then
            EchoForAudio(transform.translate,breakSE,0.5)
            echoCharge = 0.0
        end 
    end

    if echoCharge >= echoMaxCharge then
        canEcho = true
        echoCharge = echoMaxCharge
    else
        canEcho = false
    end 
end

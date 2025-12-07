echoCharge = 0.0
local breakSE = QFE.Audio.LoadSound("Slash.wav")

function Init()
    echoCharge = 2.0
end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
            EchoForAudio(transform.translate,breakSE,0.5)
            echoCharge = echoCharge -0.1
            DebugLog(echoCharge)
    end
end

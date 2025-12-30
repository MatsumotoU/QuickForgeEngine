local breakSE = QFE.Audio.LoadSound("Down.wav")

function Init()

end

function Update()

end

function OnCollisionEnter(id,obj)
    EchoForAudio(transform.translate,breakSE,0.3)
    destroy()
end
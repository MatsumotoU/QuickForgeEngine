local breakSE = QFE.Audio.LoadSound("Down.wav")

function Init()

end

function Update()

end

function OnCollisionEnter(id,obj)
    QFE.Audio.PlaySound(breakSE,false,0.3)
    destroy()
end
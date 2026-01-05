local breakSE = QFE.Audio.LoadSound("Down.wav")
hp = 1

function Init()

end

function Update()

end

function OnCollisionEnter(id,obj)
    QFE.Audio.PlaySound(breakSE,false,0.3)
    hp = hp-1
    if hp <= 0 then
        destroy()
    end
end
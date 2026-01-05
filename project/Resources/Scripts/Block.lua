local breakSE = QFE.Audio.LoadSound("Down.wav")
hp = 1
local baseHp = hp

function Init()
    baseHp = hp
end

function Update()

end

function OnCollisionEnter(id,obj)
    QFE.Audio.PlaySound(breakSE,false,0.3)
    hp = hp-1
    if hp <= 0 then
        SetSceneGlobalData("Score",GetSceneGlobalData("Score") + 100 * baseHp)
        RunAllFunction("UpdateScore")
        destroy()
    end
end
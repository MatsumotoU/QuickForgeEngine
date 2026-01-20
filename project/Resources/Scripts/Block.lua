local breakSE = QFE.Audio.LoadSound("Down.wav")
local deathSE = QFE.Audio.LoadSound("zubashu.mp3")
hp = 1
local baseHp = hp

local scaleX = 0.0
local scaleZ= 0.0

local hitInterval = 0.0

function Init()
    baseHp = hp
    scaleX = transform.scale.x
    scaleZ= transform.scale.z
end

function Update()
    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleZ,0.1)

    if hitInterval > 0.0 then
        hitInterval = hitInterval - GetDeltaTime()
    end
end

function OnCollisionEnter(id,obj)
    if hitInterval > 0.0 then
        return
    end

    
    hp = hp-1
    transform.scale.x = scaleX * 1.3
    transform.scale.z = scaleZ * 1.3
    if hp <= 0 then
        SetSceneGlobalData("Score",GetSceneGlobalData("Score") + 100 * baseHp)
        SetScore(GetScore() + 100 * baseHp)
        RunAllFunction("UpdateScore")
        local a = SimpleCreateEntity("SlashEffect.json")
        SetTranslate(a,transform.translate)
        QFE.Audio.PlaySound(deathSE,false,0.8)
        destroy()
    else
        QFE.Audio.PlaySound(breakSE,false,0.3)
        local a = SimpleCreateEntity("SmallSlash.json")
        SetTranslate(a,transform.translate)
    end
    hitInterval = 0.1
end
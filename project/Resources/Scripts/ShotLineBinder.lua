local shotLineId = 0

function Init()
    shotLineId = GetEntity("ShotLine")
end

function Update()
    if GetEntityScriptGlobal(this.GetEntityId(),"BulletShot.lua","isCanShot") and GetEntityScriptGlobal(this.GetEntityId(),"ShotGunPlayer.lua","damageAnimTime") <= 0.0 then
        RunEntityScriptFunction(shotLineId,"ShotLine.lua","OnDraw")
    else
        RunEntityScriptFunction(shotLineId,"ShotLine.lua","OffDraw")
    end
end

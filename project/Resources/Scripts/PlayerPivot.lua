playerName = "ShotGunPlayer" 
local id = 0

function Init()
    id = GetEntity(playerName)
end

function Update()
    transform.translate.x = GetTransform(id).translate.x
    transform.translate.z = GetTransform(id).translate.z
    transform.rotate.y = QFE.Math.SimpleEaseIn(transform.rotate.y,GetTransform(id).rotate.y,0.3) 
end

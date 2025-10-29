playerName = "ShotGunPlayer" 
local id = 0

function Init()
    id = GetEntity(playerName)
end

function Update()
    transform.translate = GetTransform(id).translate
    transform.rotate.y = QFE.Math.SimpleEaseIn(transform.rotate.y,GetTransform(id).rotate.y,0.3) 
end

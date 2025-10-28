local playerId = 0
playerName = "ShotGunPlayer"

function Init()
    playerId = GetEntity(playerName)
end

function Update()
    transform.translate = GetTransform(playerId).translate
    transform.translate.y = 1.0
    transform.translate.z = transform.translate.z + 0.5
    local pumpAction = GetEntityScriptGlobal(playerId,"BulletShot.lua","shotInterval")
    local maxPumpAction = GetEntityScriptGlobal(playerId,"BulletShot.lua","maxShotInterval")
    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,pumpAction/maxPumpAction,0.5)
    if transform.scale.x <= 0.0 then
        transform.scale.x = 0.0
    end
end

local playerId = 0
playerName = "ShotGunPlayer"

function Init()
    playerId = GetEntity(playerName)
end

function Update()
    if GetEntityScriptGlobal(playerId,"BulletShot.lua","bullets") < GetEntityScriptGlobal(playerId,"BulletShot.lua","maxBullets") then
        if QFE.Input.GetKeyPress("MoveLeft") or QFE.Input.GetGamePadLeftStickDir().x < -0.5 then
            transform.translate = GetTransform(playerId).translate
            transform.translate.y = 1.0
            transform.translate.z = transform.translate.z + 0.5
            local pumpAction = GetEntityScriptGlobal(playerId,"BulletShot.lua","reloadInterval")
            local maxPumpAction = GetEntityScriptGlobal(playerId,"BulletShot.lua","targetReloadInterval")
            transform.scale.x = 1.0 - (pumpAction/maxPumpAction)
            if transform.scale.x <= 0.0 then
                transform.scale.x = 0.0
            end
            return
        end
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.5)

end


-- プレイヤーの位置を取得
playerName = "ShotGunPlayer"
local playerID = 0

local offsetX = 18.0

function Init()
    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)
end

function Update()

     local targetTransform = GetTransform(playerID)

     local endLinePosX = targetTransform.translate.x - offsetX

     if transform.translate.x <= endLinePosX or transform.translate.x > targetTransform.translate.x + 20.0 then
        destroy()
     end
end

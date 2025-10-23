
-- プレイヤーの位置を取得
playerName = "Camera"
local playerID = 0

local offsetX = 36.0

function Init()
    -- プレイヤーのIDを取得
    playerID = GetEntity(playerName)
end

function Update()
    transform.scale.x = 2.0
    transform.scale.y = 1.0
    transform.scale.z = 2.0

     local targetTransform = GetTransform(playerID)

     local endLinePosX = targetTransform.translate.x - offsetX

     if transform.translate.x <= endLinePosX or transform.translate.x > targetTransform.translate.x + 40.0 then
        destroy()
     end
end

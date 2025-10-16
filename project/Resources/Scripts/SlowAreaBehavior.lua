
-- スローの生成時間カウンター
local lifeTimer = 0.0
-- スローの生成時間(秒)
lifeMaxTime = 3.0

--[[
    初期化処理
--]]
function Init()
    lifeTimer = 0.0
end

--[[
    更新処理
--]]
function Update()
    lifeTimer = lifeTimer + 1.0 / (60.0 * lifeMaxTime)

    -- 時間がたったらスローが終了
    if lifeTimer >= 1.0 then
        destroy()
    end
end

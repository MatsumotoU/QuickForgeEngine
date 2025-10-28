
-- 現在のチュートリアルの進行度合いを取得
TutorialManagerJson = "TutorialManager"
local ID = 0
local TutorialScriptName = "TutorialManager.lua"
local eventTypeVarName = "EventType"

local targetId = 5

function Init()
     -- チュートリアルIDを取得
    ID = GetEntity(TutorialManagerJson)
end

function Update()

    local eType = GetEntityScriptGlobal(ID,TutorialScriptName,eventTypeVarName)

    if targetId == eType then
        destroy()
    end
end

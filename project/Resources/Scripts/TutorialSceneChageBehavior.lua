-- 現在のチュートリアルの進行度合いを取得
TutorialManagerJson = "TutorialManager"
local ID = 0
local TutorialScriptName = "TutorialManager.lua"
local eventTypeVarName = "EventType"

local mat = Material.new()

local timer = 0.0
local maxTime = 1.0

function Init()

    -- チュートリアルIDを取得
    ID = GetEntity(TutorialManagerJson)
    mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function Update()

     local eType = GetEntityScriptGlobal(ID,TutorialScriptName,eventTypeVarName)

    if eType >= 7 then

        local deltatime = GetDeltaTime()
        timer = timer + deltatime

        -- 0から1の範囲に収める
        local t = math.min(timer / maxTime,1.0)

        mat.color.w = QFE.Math.Leap(1.0,0.0,t)

        if timer >= maxTime then
            LoadScene("TitleScene")
        end
    end
end


-- 現在のチュートリアルの進行度合いを取得
TutorialManagerJson = "TutorialManager"
local ID = 0
local TutorialScriptName = "TutorialManager.lua"
local eventTypeVarName = "EventType"

-- 指定したイベントの時に表示されるようにする
targetType = 0
shotType = 0
local shotTypeVarName = "shotType"

-- UIの種類
UIType = 0

function Init()
 -- チュートリアルIDを取得
ID = GetEntity(TutorialManagerJson)
end

function Update()

   local eType = GetEntityScriptGlobal(ID,TutorialScriptName,eventTypeVarName)

--    DebugLog("LinkedID:"eType)
--    DebugLog("targetType :"..targetType)

   -- 指定したタイプと会わなければ削除
   if eType ~= targetType then
        destroy()
   end

   if UIType == 1 then
        local eSType = GetEntityScriptGlobal(ID,TutorialScriptName,shotTypeVarName)
        if eSType ~= shotType then
            destroy()
        end
   end

end

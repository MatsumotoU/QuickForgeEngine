
-- マップ情報を取得するための名前
mapObjName = "MapGenerator"
generatorMapScriptName = "MapGenerator.lua"
varMapName = "map"
local linkID = 0

-- 取得したマップ
local map = {{},{}}
-- ブロックの大きさ
local kBlockSize = 1.0

function Init()
    -- 生成したマップを取得
    linkID = GetEntity(mapObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog("StartLoadMapData")
    DebugLog(generatorMapScriptName)
    DebugLog(varMapName)
    map = GetEntityScriptGlobal(linkID,generatorMapScriptName,varMapName)

    
end

function Update()

end

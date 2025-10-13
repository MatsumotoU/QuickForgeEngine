
-- マップオブジェクトの名前
wallObjName = "name"

-- マップの縦、横幅
maxWidth = 100
maxHeight = 11

-- ブロックの大きさ
blockSize = 1.0

-- マップ
local map = {{},{}}

function Init()
    DebugLog("mapInit")
    GenerateMap(maxWidth,maxHeight)
    SpawnMap(map)
end

function Update()

end

-- マップを生成する
function GenerateMap(width,height)
for z = 1,height do
    map[z] = {}
    for x = 1, width do
        -- 簡易的にマップを生成
        local r = math.random(1,2)
        if r == 2 then
            map[z][x] = 0
        else
            map[z][x] = 1
        end
    end
end
end

-- マップオブジェクトを出現させる
function SpawnMap(map)
    for z = 1,#map do
        for x = 1,#map[z] do
            -- マップが空白の場合は早期リターン
            if map[z][x] ~= 0 then
            
            -- マップの位置を生成
            local xPos = (x - 1) * blockSize
            local zPos = (z - 1) * blockSize
            local tmpTransform = Transform.new()
            tmpTransform.translate.x = xPos
            tmpTransform.translate.y = 0.0
            tmpTransform.translate.z = zPos

                if map[z][x] == 1 then
                    CreateEntity(wallObjName,tmpTransform)
                end
            end
        end
    end
end

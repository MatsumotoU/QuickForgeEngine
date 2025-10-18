local map = {{},{}}

function Init()
    map = Load2DMap("testCSV.csv")
    for x = 1, #map, 1 do
        for y = 1, #map[x], 1 do
            DebugLog(map[x][y])
        end
    end

end

function Update()

end

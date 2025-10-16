map = {{},{}}

function Init()
    map[1][1] = 0
    map[1][2] = 1
    map[2][1] = 2
    map[2][2] = 3

    DebugLog("Source")
    for i = 1, #map, 1 do
       local row = map[i]
       for j = 1, #row, 1 do
            DebugLog(map[i][j])
       end
    end
end

function Update()
    if QFE.Input.GetKeyTrigger("Decide") then
        DebugLog("Source")
        for i = 1, #map, 1 do
       local row = map[i]
       for j = 1, #row, 1 do
            DebugLog(map[i][j])
       end
    end
    end
end

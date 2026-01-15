function Init()
    DebugLog("SpawnMoveXEntity.lua Initialized")
    -- MovingCube.json を生成する
    local id = SimpleCreateEntity("MovingCube.json")
    DebugLog("Spawned MovingCube with ID: " .. tostring(id))
end

function Update()
    -- 特に何もしない
end

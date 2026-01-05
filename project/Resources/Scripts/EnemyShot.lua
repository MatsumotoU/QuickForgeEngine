shotInterval = 2.0
timer = 0.0

function Init()
    timer = 0.0
end

function Update()
    local delta = GetDeltaTime()
    timer = timer + delta

    if timer >= shotInterval then
        timer = 0.0
        Shoot()
    end
end

function Shoot()
    -- Get Player Position
    local playerID = GetEntity("PlayerBar")
    if playerID == -1 then
        return
    end

    local playerTransform = GetTransform(playerID)
    local myTransform = transform
    
    local playerPos = playerTransform.translate
    local myPos = myTransform.translate

    -- Calculate Direction
    local dir = Vector3.new(playerPos.x - myPos.x, playerPos.y - myPos.y, playerPos.z - myPos.z)
    dir:Normalize()

    -- Spawn Bullet
    local bulletID = SimpleCreateEntity("EnemyBullet.json")
    
    -- Set Bullet Position
    SetTranslate(bulletID, myPos)

    -- Set Bullet Rotation to look at Player
    local rot = QFE.Math.LookAtFromDir(dir)
    SetRotate(bulletID, rot)
end

shotInterval = 1.0
timer = 0.0

function Init()
    timer = 0.0
end

function Update()
    if CountEntityTag("StageStop") > 0 then
        return
    end

    local delta = GetDeltaTime()
    timer = timer + delta

    if timer >= shotInterval then
        timer = 0.0
        Shoot()
    end
end

function Shoot()
    -- Base direction is -Z
    -- We want 3-way: Center, Left, Right
    -- Assuming Y is up, rotate around Y.
    
    -- Center (0, 0, -1)
    local centerDir = Vector3.new(0, 0, -1)
    
    -- Angle for spread (e.g. 30 degrees)
    local angle = 30
    local rad = math.rad(angle)
    local sinVal = math.sin(rad)
    local cosVal = math.cos(rad)

    -- Left: Rotate (0, 0, -1) by +angle (counter-clockwise) or -angle?
    -- x' = x*cos - z*sin (standard 2D rotation) ??
    -- Let's just do simple math.
    -- (0, 0, -1) rotated by theta around Y.
    -- x = sin(theta), z = -cos(theta)
    
    local leftDir = Vector3.new(math.sin(-rad), 0, -math.cos(-rad))
    local rightDir = Vector3.new(math.sin(rad), 0, -math.cos(rad))

    SpawnBullet(centerDir)
    SpawnBullet(leftDir)
    SpawnBullet(rightDir)
end

function SpawnBullet(dir)
    dir:Normalize()
    local bulletID = SimpleCreateEntity("EnemyBullet.json")
    
    local myPos = transform.translate
    SetTranslate(bulletID, myPos)

    local rot = QFE.Math.LookAtFromDir(dir)
    SetRotate(bulletID, rot)
end

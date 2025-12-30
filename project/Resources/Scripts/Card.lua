cardName = "brankCard"

local spawnTransform = Transform.new()
local time = 0.0

function Init()
    spawnTransform = transform
end

function Update()
    time = time + 0.3
    transform.translate.y = spawnTransform.translate.y + (math.sin(time) * 0.3)

end

function Select()
    RunAllFunction("OnUpGrade" .. cardName)
    destroy()
end

function NoSelect()
    destroy()
end
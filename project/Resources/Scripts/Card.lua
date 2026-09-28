cardName = "brankCard"

local spawnY = 0.0
local time = 0.0

local selectSE = QFE.Audio.LoadSound("Slash.wav")

function Init()
    spawnY = transform.translate.y
    time = 0.0
end

function Update()
    time = time + 0.3
    transform.translate.y = spawnY + (math.sin(time) * 0.3)

end

function Select()
    QFE.Audio.PlaySound(selectSE,false,0.5)
    RunAllFunction("OnUpGrade" .. cardName)
    destroy()
end

function NoSelect()
    destroy()
end

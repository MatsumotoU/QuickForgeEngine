intVal = 5
floatVal = 1.0
stringVal = "HelloWorld!"
intVal2 = 10

local aObj = 0

function Init()
    DebugLog("Start")
    DebugLog(QFE.Math.Rand(0.0,100.0))
    aObj = GetEntity("a")
    DebugLog("End")
end

function Update()
    DebugLog(aObj)
    DebugLog(GetForce(aObj).velocity.y)
    DebugLog("EndUpdate")
end
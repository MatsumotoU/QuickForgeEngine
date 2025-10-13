speed = 0.1
stateLog = "Update"
global = 0.0

function Init()
	DebugLog(speed)
	transform.translate.x = 0
	DebugLog("Init")
end

function Update()
	DebugLog(speed)
	transform.translate.x = transform.translate.x + speed
	DebugLog(stateLog)
end

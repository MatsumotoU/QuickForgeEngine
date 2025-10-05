speed = 0.1

function Init()
	DebugLog(speed)
	transform.translate.x = 0
	DebugLog("Init")
end

function Update()
	DebugLog(speed)
	transform.translate.x = transform.translate.x + speed
	DebugLog("Update")
end

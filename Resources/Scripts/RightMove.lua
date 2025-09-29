function Init()
	DebugLog("RightMove Init")
	transform.translate.x = 0
	DebugLog("Done")
end

function Update()
	DebugLog("RightMove")
	transform.translate.x = transform.translate.x + 0.1
	DebugLog("Done")
end

function Init()
	DebugLog("Init")
end

function Update()
	DebugLog("Update")
	this.transform.translate.x = this.transform.translate.x - 0.01
end

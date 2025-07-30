function Init()
	DebugLog("Init")
end

function Update()
	DebugLog("Update")
	local left = input:GetKeyMoveDir()
	this.transform.translate.x = this.transform.translate.x + left.x * 0.1
	this.transform.translate.y = this.transform.translate.y + left.y * 0.1
end

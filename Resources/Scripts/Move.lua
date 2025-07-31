function Init()
	DebugLog("Init")
end

function Update()
	DebugLog("Update")
	local left = input:GetKeyMoveDir()
	this.transform.translate.x = this.transform.translate.x + left.x * deltaTime
	this.transform.translate.y = this.transform.translate.y + left.y * deltaTime
end

function Collision()
	DebugLog("IsHit")
	sceneManager:LoadScene("Title")
end


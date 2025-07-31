function Init()
	DebugLog("Init")
end

function Update()
	DebugLog("Update")

	local left = input:GetKeyMoveDir()
	DebugLog(left.x)
	DebugLog(left.y)
	local cnt = controller:GetLeftStick(0)
	DebugLog(cnt.x)
	DebugLog(cnt.y)
	local move = left
	move.x = move.x + cnt.x
	move.y = move.y + cnt.y
	local move = move:Normalize()
	DebugLog(move.x)
	DebugLog(move.y)

	this.transform.translate.x = this.transform.translate.x + move.x * deltaTime
	this.transform.translate.y = this.transform.translate.y + move.y * deltaTime
end

function Collision()
	DebugLog("IsHit")
	sceneManager:LoadScene("Title")
end


function Init()
	DebugLog("Init")
end

function Update()
	DebugLog("Update")
	transform.translate.x = transform.translate.x + QFE.Input.GetKeyMoveDir().x * deltaTime
	transform.translate.y = transform.translate.y + QFE.Input.GetKeyMoveDir().y * deltaTime
end
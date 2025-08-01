function Init()

end

function Update()

	local left = input:GetKeyMoveDir()
	local cnt = controller:GetLeftStick(0)
	local move = left
	move.x = move.x + cnt.x
	move.y = move.y + cnt.y
	local move = move:Normalize()

	this.transform.translate.x = this.transform.translate.x + move.x * deltaTime
	this.transform.translate.y = this.transform.translate.y + move.y * deltaTime

	if this.transform.translate.x > 1.0 then this.transform.translate.x = 1.0 end
	if this.transform.translate.x < -1.0 then this.transform.translate.x = -1.0 end
	if this.transform.translate.y > 0.5 then this.transform.translate.y = 0.5 end
	if this.transform.translate.y < -0.5 then this.transform.translate.y = -0.5 end
end

function Collision()

	sceneManager:LoadScene("Title")
end


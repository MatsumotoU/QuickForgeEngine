speed = 1.0
function Init()

end

function Update()
	if QFE.Input.GetKeyPress("MoveUp") then
		transform.translate.z = transform.translate.z + speed * 0.03
	end

	if QFE.Input.GetKeyPress("MoveDown") then
		transform.translate.z = transform.translate.z - speed * 0.03
	end

	if QFE.Input.GetKeyPress("MoveRight") then
		transform.translate.x = transform.translate.x + speed * 0.03
	end

	if QFE.Input.GetKeyPress("MoveLeft") then
		transform.translate.x = transform.translate.x - speed * 0.03
	end
end
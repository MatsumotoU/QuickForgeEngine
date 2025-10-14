speed = 1.0
function Init()

end

function Update()
	if QFE.Input.GetKeyPress(DIK_W) then
		transform.translate.z = transform.translate.z + speed * 0.016
	end

	if QFE.Input.GetKeyPress(DIK_S) then
		transform.translate.z = transform.translate.z - speed * 0.016
	end

	if QFE.Input.GetKeyPress(DIK_D) then
		transform.translate.x = transform.translate.x + speed * 0.016
	end

	if QFE.Input.GetKeyPress(DIK_A) then
		transform.translate.x = transform.translate.x - speed * 0.016
	end
end
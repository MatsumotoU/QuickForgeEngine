rightCount = 0
leftCount = 0
topCount = 0
underCount = 0
maxCount = 0

function Init()
rightCount = 0
leftCount = 0
topCount = 0
underCount = 0
maxCount = 0
end

function Update()
	if QFE.Input.GetKeyTrigger(DIK_W) then
		transform.translate.z = transform.translate.z + 1.0
		if topCount <= 0 then
			rightCount = 0
			leftCount = 0
			underCount = 0
		end
		topCount = topCount + 1
	end

	if QFE.Input.GetKeyTrigger(DIK_S) then
		transform.translate.z = transform.translate.z - 1.0
		if underCount <= 0 then
			rightCount = 0
			leftCount = 0
			topCount = 0
		end
		underCount = underCount + 1
	end

	if QFE.Input.GetKeyTrigger(DIK_D) then
		transform.translate.x = transform.translate.x + 1.0
		if rightCount <= 0 then
			underCount = 0
			leftCount = 0
			topCount = 0
		end
		rightCount = rightCount + 1
	end

	if QFE.Input.GetKeyTrigger(DIK_A) then
		transform.translate.x = transform.translate.x - 1.0
		if leftCount <= 0 then
			rightCount = 0
			underCount = 0
			topCount = 0
		end
		leftCount = leftCount + 1
	end

	maxCount = rightCount + leftCount + underCount + topCount

	transform.scale.x = 1.5 + maxCount * 0.8
	transform.scale.y = 1.5 + maxCount * 0.8
	transform.scale.z = 1.5 + maxCount * 0.8
end
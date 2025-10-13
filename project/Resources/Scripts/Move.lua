rightCount = 0
leftCount = 0
topCount = 0
underCount = 0
maxCount = 0
isAttacking = false
attackChargeTime = 0.0
maxAttackChargeTime = 1.0
isAllive = true

giveDamageTime = 0.0
maxGiveDamageTime = 0.1
isGiveDamage = false

function Init()
rightCount = 0
leftCount = 0
topCount = 0
underCount = 0
maxCount = 0
isAttacking = false
attackChargeTime = 0.0
isAllive = true
end

function Update()
	if giveDamageTime > 0.0 then
		giveDamageTime = giveDamageTime - 0.016
	else
		isGiveDamage = false
	end
	
	if not isAllive then
		transform.scale.y = 0.1
		return
	end

	if attackChargeTime > 0.0 then
		attackChargeTime = attackChargeTime - 0.016
		transform.translate.y = 1.0
	else
		if isAttacking then
			isAttacking = false
			transform.translate.y = 0.0
			isGiveDamage = true
			giveDamageTime = maxGiveDamageTime
		end
	end

	if not isAttacking then
		if QFE.Input.GetKeyTrigger(DIK_W) then
			transform.translate.z = transform.translate.z + 1.0
			if topCount <= 0 then
				rightCount = 0
				leftCount = 0
				underCount = 0
			end
			topCount = topCount + 1
			attack()
		end

		if QFE.Input.GetKeyTrigger(DIK_S) then
			transform.translate.z = transform.translate.z - 1.0
			if underCount <= 0 then
				rightCount = 0
				leftCount = 0
				topCount = 0
			end
			underCount = underCount + 1
			attack()
		end

		if QFE.Input.GetKeyTrigger(DIK_D) then
			transform.translate.x = transform.translate.x + 1.0
			if rightCount <= 0 then
				underCount = 0
				leftCount = 0
				topCount = 0
			end
			rightCount = rightCount + 1
			attack()
		end

		if QFE.Input.GetKeyTrigger(DIK_A) then
			transform.translate.x = transform.translate.x - 1.0
			if leftCount <= 0 then
				rightCount = 0
				underCount = 0
				topCount = 0
			end
			leftCount = leftCount + 1
			attack()
		end
	end

	maxCount = rightCount + leftCount + underCount + topCount

	transform.scale.x = 1.5 + maxCount * 0.8
	transform.scale.y = 1.5 + maxCount * 0.8
	transform.scale.z = 1.5 + maxCount * 0.8
end

function attack()
	isAttacking = true
	attackChargeTime = maxAttackChargeTime
	transform.translate.y = 1.0
end
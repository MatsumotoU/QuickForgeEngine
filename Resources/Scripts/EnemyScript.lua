enemyReversPoint = 1
isReverse = false
isReadyChack = true
isAlive = true

attackInterval = 0.0
maxAttackInterVal = 1.0
isAttaking = false

attackChargeTime = 0.0
maxAttackChargeTime = 1.0

reverseInterval = 0.0
maxReveseInterVal = 0.3

playerObj = 0

function Init()
transform.scale.x = 0.5 + enemyReversPoint * 0.1
transform.scale.z = 0.5 + enemyReversPoint * 0.1
playerObj = GetEntity("Player")
isAlive = true
isAttaking = false
end

function Update()
    if reverseInterval > 0.0 then
        reverseInterval = reverseInterval -0.016
    end

    if isAttaking then
        transform.translate.y = 0.5
    else
        transform.translate.y = 0.0
    end

    if attackChargeTime > 0.0 then
        attackChargeTime = attackChargeTime -0.016
    else 
        isAttaking = false
    end

-- 反転しているかのフラグ
    if isReverse then
        transform.rotate.y = 3.14 * 0.25
    else
        transform.rotate.y = 0.0
    end
-- 反転するかどうか
    if QFE.Input.GetKeyTrigger(DIK_W) then
		isReadyChack = true
	end
    if QFE.Input.GetKeyTrigger(DIK_S) then
		isReadyChack = true
	end
    if QFE.Input.GetKeyTrigger(DIK_A) then
		isReadyChack = true
	end
    if QFE.Input.GetKeyTrigger(DIK_D) then
		isReadyChack = true
	end

    -- 敵が裏返っていたら攻撃はしない
    if isReverse then
        isAttaking = false
        return
    end
    -- 攻撃クールダウン
    if attackInterval > 0.0 then
        attackInterval = attackInterval -0.016
        return
    end

    Attack()
end

function OnCollisionStay(idA,idB,objA)
    if objA.name == "Player" then
        if not GetEntityScriptGlobal(idA,"Move.lua","isGiveDamage") then
            return
        end

        local isPlayerAttaking = GetEntityScriptGlobal(idA,"Move.lua","isAttacking")

        if isReadyChack == true and not isPlayerAttaking then
            local count = GetEntityScriptGlobal(idA,"Move.lua","maxCount")
            DebugLog(count)
            if enemyReversPoint <= count then
                if reverseInterval <= 0.0 then
                    isReverse = not isReverse
                    DebugLog("isReverse!")
                    reverseInterval = maxReveseInterVal
                end
            else
                if isAttaking then
                    isReverse = not isReverse
                end
            end
            DebugLog("EndCollision")
            isReadyChack = false
        end
    end
end

function Attack ()
    local playerTransform = Transform.new()
    playerTransform = GetTransform(playerObj)

    local temp = Transform.new()
    local tempX = playerTransform.translate.x - transform.translate.x
    local tempZ = playerTransform.translate.z - transform.translate.z

    if tempX < -0.5 then
        transform.translate.x = transform.translate.x - 1.0
    end
    if tempX > 0.5 then
        transform.translate.x = transform.translate.x + 1.0
    end
    if tempZ < -0.5 then
        transform.translate.z = transform.translate.z - 1.0
    end
    if tempZ > 0.5 then
        transform.translate.z = transform.translate.z + 1.0
    end

    attackInterval = maxAttackInterVal
    isAttaking = true
    attackChargeTime = maxAttackChargeTime
end

-- function OnCollisionEnter(idA,idB)
-- local count = GetEntityScriptGlobal(idA,"Move.lua","maxCount")
-- DebugLog(count)
-- if enemyReversPoint <= count then
--     isReverse = not isReverse
--     DebugLog("isReverse!")
-- end
-- DebugLog("EndCollision")
-- end
enemyReversPoint = 1
isReverse = false
isReadyChack = true

function Init()
transform.scale.x = 0.5 + enemyReversPoint * 0.1
transform.scale.z = 0.5 + enemyReversPoint * 0.1
end

function Update()
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

end

function OnCollisionStay(idA,idB)
    if isReadyChack == true then
        local count = GetEntityScriptGlobal(idA,"Move.lua","maxCount")
        DebugLog(count)
        if enemyReversPoint <= count then
            isReverse = not isReverse
            DebugLog("isReverse!")
        end
        DebugLog("EndCollision")
        isReadyChack = false
    end
end

function func()

if a<=5 then
    

else

end

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
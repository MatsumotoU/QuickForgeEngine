local moveSpeed = 4.5
local moveAcc = 0.7
local dashSpeed = 12.0
local damageInterval = 0.0

local moveTime = 0.0
local isStart = false
local scaleX = 0.0
local scaleY = 0.0
local isNearBall = true

local isMeshHeart = false

local strongBeatSE = QFE.Audio.LoadSound("StrongBeat.wav")
local moveRotateY = 0.0

local beatId = 0

function Init()
    scaleX = transform.scale.x
    scaleY = transform.scale.z
    beatId = GetEntity("Pacemaker")
end

function Update()
    if GetEntityScriptGlobal(beatId,"Pacemaker.lua","bpm") <= 0 then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.scale.y = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.02)
        transform.rotate.y =transform.rotate.y+1.0
        return
    end

    if damageInterval > 0.0 then
        damageInterval = damageInterval - GetDeltaTime()
        transform.scale.x = math.abs(math.sin(damageInterval*10.0)) 
        transform.scale.z = math.abs(math.cos(damageInterval*10.0))
    end

    if QFE.Input.GetKeyTrigger("Jump") then
        if CountEntityTag("card") <= 0 then
            isStart =true
        end
    end

    if isNearBall then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX*0.2,0.3)
    else
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,scaleX,0.3)
    end
    aabbCollider.aabb.size.x = transform.scale.x*0.8
    
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,scaleY,0.1)
    transform.rotate.y = QFE.Math.SimpleEaseIn(transform.rotate.y+moveRotateY,3.14,0.1)

    if transform.scale.x <= 0.75 then
        if not isMeshHeart then
            ChangeMesh(GetThisEntityId(),"heart.obj")
            isMeshHeart = true
        end        
    else
        if isMeshHeart then
            ChangeMesh(GetThisEntityId(),"Box1x1.obj")
            isMeshHeart = false
        end
    end

    if not isStart then
        return 
    end
    -- ダッシュ
    if QFE.Input.GetKeyTrigger("Jump") then
        if QFE.Input.GetKeyPress("MoveRight") then
        force.velocity.x = dashSpeed
        
        elseif QFE.Input.GetKeyPress("MoveLeft") then
        force.velocity.x = -dashSpeed
        end
    end

    -- 移動
    local isMove = false
    if QFE.Input.GetKeyPress("MoveRight") then
        if force.velocity.x < moveSpeed then
            force.velocity.x = force.velocity.x + moveAcc
        end
        moveTime = moveTime + 1.0
        moveRotateY = 0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveLeft") then
        if force.velocity.x > -moveSpeed then
            force.velocity.x = force.velocity.x - moveAcc
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveDown") then
        if force.velocity.z > -moveSpeed *0.5 then
            force.velocity.z = force.velocity.z - moveAcc*0.5
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if QFE.Input.GetKeyPress("MoveUp") then
        if force.velocity.z < moveSpeed * 0.5 then
            force.velocity.z = force.velocity.z + moveAcc*0.5
        end
        moveTime = moveTime + 1.0
        moveRotateY = -0.05
        isMove = true
    end
    if not isMove then
        force.velocity.x = force.velocity.x *0.8
        force.velocity.z = force.velocity.z *0.8
        moveRotateY = 0.0
    end

    if moveTime > 20 then
        --Echo(transform.translate,0.8)
        moveTime = 0.0
    end

    if transform.translate.y ~= 0.0 then
        transform.translate.y = 0.0
    end


    if GetMinLengthToEntityFromTag("ball",transform.translate) >= 2.5 then
        isNearBall = true
    else
        isNearBall = false
    end
end

function OnCollisionEnter(id,obj)
    force.velocity.x = 0.0
    if obj.tag == "ball" then
        local x = (GetTransform(id).translate.x - transform.translate.x)
        transform.rotate.y = x * 10.0;
    end
end

function OnCollisionStay(id,obj)
    if obj.tag == "enemyBullet" or obj.tag == "Enemy" then
        if damageInterval > 0.0 then
            return
        end

        local pacemakerId = GetEntity("Pacemaker")
        if pacemakerId ~= -1 then
            local currentBpm = GetEntityScriptGlobal(pacemakerId, "Pacemaker.lua", "bpm")
            if currentBpm then
                SetEntityScriptGlobal(pacemakerId, "Pacemaker.lua", "bpm", currentBpm - 30)
                DebugLog("Player Hit! BPM Reduced to: " .. tostring(currentBpm - 30))
                damageInterval = 2.5
            end
        end
        force.velocity.x = (transform.translate.x - GetTransform(id).translate.x)*30.0
        force.velocity.z = (transform.translate.z - GetTransform(id).translate.z)*30.0
    end
end

function OnStrongBeat()
    --transform.scale.x = scaleX * 1.2
    transform.scale.z = scaleY * 1.1
end

function OnBar()
    QFE.Audio.PlaySound(strongBeatSE,false,0.5)
    --transform.scale.x = scaleX * 1.1
    transform.scale.z = scaleY * 1.2
end

function OnNextStage()
    isStart = false
    transform.translate.x = 0.0
    transform.translate.z = -4.0
    force.velocity.x = 0.0
end
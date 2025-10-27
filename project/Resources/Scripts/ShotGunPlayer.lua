moveSpeed = 0.5
backSpeed = 5.0
local objectDir = Vector3.new(0.0,0.0,0.0)
isBackFriping = false
minVelocity = 1.0

local deltatime = 0.016
local time = 0.0

function Init()
objectDir.x = 0.0
objectDir.y = 0.0
objectDir.z = 0.0
isBackFriping = false
end

function Update()
    deltatime = GetDeltaTime()
    time = time + deltatime
    -- アニメーション
    transform.scale.y = 1.0 - (math.sin(time * 2.0) * 0.3)
    transform.rotate.x = QFE.Math.SimpleEaseIn(transform.rotate.x,0.0,0.1)

    -- 接地処理
    if transform.translate.y <= 0.0 then
        transform.translate.y = 0.0
        force.velocity.y = 0.0
        force.acceleration.y = 0.0
    end

    -- 自分に力がかかっていたら移動できない
    if force.velocity:Length() > minVelocity then
        return
    end

    -- 自分のコッキング時間中は動けない
    if GetEntityScriptGlobal(this.GetEntityId(),"BulletShot.lua","shotInterval") > 0.0 then
        return
    end 

    -- 移動方向に向きを設定する
    local moveDir = QFE.Input.GetKeyMoveDir()
    moveDir.x = moveDir.x + QFE.Input.GetGamePadLeftStickDir().x
    moveDir.y = moveDir.y + QFE.Input.GetGamePadLeftStickDir().y
    moveDir = moveDir:Normalize()
    if moveDir:Length() >= 0.1 then
        if not isBackFriping then
            objectDir = moveDir  
            transform.rotate.x = moveDir.x * 0.1
            transform.rotate.x = moveDir.y * 0.1
            if objectDir.x <= 0.0 then
                objectDir.x = 0.0
            end
        end
    end    
    -- transform.rotate.z = QFE.Math.SimpleEaseIn(transform.rotate.z,0.0,0.1)

    -- バックフリップ終了処
    if force.velocity:Length() <= minVelocity then
        force.velocity.y = 0.0
        force.velocity.z = 0.0
        force.velocity.x = 0.0
        if isBackFriping then
           isBackFriping = false
           
        end
    end

    -- バックフリップ開始処理
    if QFE.Input.GetKeyPress("MoveLeft") or moveDir.x + QFE.Input.GetGamePadLeftStickDir().x < -0.5 then
       transform.translate.x = transform.translate.x - (moveSpeed*0.25 * deltatime)
       objectDir.x = 1.0
       objectDir.y = 0.0
       -- 設定された向きを見る
       transform.rotate.y =QFE.Math.SimpleEaseIn(transform.rotate.y,math.atan(objectDir.x,objectDir.y),0.8)
       return
    end

    -- 設定された向きを見る
    transform.rotate.y =QFE.Math.SimpleEaseIn(transform.rotate.y,math.atan(objectDir.x,objectDir.y),0.8)

    -- 移動処理
    if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetKeyPress("MoveDown") or QFE.Input.GetKeyPress("MoveUp") or QFE.Input.GetGamePadLeftStickDir():Length() > 0.5 then
        if not isBackFriping then
            transform:AddForward(moveSpeed*deltatime)
        end
    end
end

function OnCollisionEnter(id,obj)
    
end

function OnCollisionStay(id,obj)
    if isBackFriping then
        return
    end

    if obj.tag == "bullet" then
        return
    end

    if force.velocity:Length() >= minVelocity then
        force.velocity.x = 0.0
        force.velocity.z = 0.0
        force.acceleration.x = 0.0
        force.acceleration.z = 0.0
        DebugLog("Reset force")
    end

end
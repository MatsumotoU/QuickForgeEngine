moveSpeed = 0.5
backSpeed = 5.0
local objectDir = Vector3.new(0.0,0.0,0.0)
isBackFriping = false
minVelocity = 1.0

local deltatime = 0.016

-- リセットに関する名前
transitionObjName = "SceneTransitionManager"
sceneTransitionScriptName = "SceneTransitionManager.lua"
varIsResetName = "isReset"
local transitionID = 0

function Init()
objectDir.x = 0.0
objectDir.y = 0.0
objectDir.z = 0.0
isBackFriping = false

    -- シーン遷移を取得
    transitionID = GetEntity(transitionObjName)
end

function Update()
    deltatime = GetDeltaTime()

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

    -- 移動方向に向きを設定する
    local moveDir = QFE.Input.GetKeyMoveDir()
    if moveDir:Length() >= 0.1 then
        if not isBackFriping then
            objectDir = moveDir  
            if objectDir.x <= 0.0 then
                objectDir.x = 0.0
            end
        end
    end    

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
    if QFE.Input.GetKeyPress("MoveLeft") then
       transform.translate.x = transform.translate.x - (moveSpeed*0.25 * deltatime)
       objectDir.x = 1.0
       objectDir.y = 0.0
       -- 設定された向きを見る
        transform.rotate.y = math.atan(objectDir.x,objectDir.y)
       return
    end

    -- 設定された向きを見る
    transform.rotate.y = math.atan(objectDir.x,objectDir.y)

    -- 移動処理
    if QFE.Input.GetKeyPress("MoveRight") or QFE.Input.GetKeyPress("MoveDown") or QFE.Input.GetKeyPress("MoveUp") then
        if not isBackFriping then
            transform:AddForward(moveSpeed*deltatime)
        end
    end

    local isReset = GetEntityScriptGlobal(transitionID,sceneTransitionScriptName,varIsResetName)
    -- リセット
    if isReset then
        Reset()
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
    end
end

function Reset()
    -- 位置をリセット
    transform.translate.x = 3.0
    transform.translate.y = 0.0
    transform.translate.z = 5.0
end
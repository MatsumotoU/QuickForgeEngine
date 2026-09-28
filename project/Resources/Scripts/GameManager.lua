local blockCount = 0
local ballCount = 0
local stage = 0
local beatId = 0

local isGameEnd = false
local isShop = false
local isNextStage = false
local isTakeFirstDamagePlayer = false

local gameEndTimer = 0.0
local cameraId = -1
local cameraBasePosition = Vector3.new(0.0, 0.0, 0.0)
local cameraShakeTimer = 0.0
local cameraShakeDuration = 0.28
local cameraShakeStrength = 0.22

local fanfareSE = QFE.Audio.LoadSound("fanfare.wav")
local missSE = QFE.Audio.LoadSound("miss.wav")

function Init()
    SetSceneGlobalData("Score",0)
    blockCount = CountEntityTag("block")
    ballCount = CountEntityTag("ball")
    beatId = GetEntity("Pacemaker")
    cameraId = GetEntity("Camera")
    if cameraId ~= -1 then
        local cameraTransform = GetTransform(cameraId)
        cameraBasePosition = Vector3.new(
            cameraTransform.translate.x,
            cameraTransform.translate.y,
            cameraTransform.translate.z)
    end
    cameraShakeTimer = 0.0
    gameEndTimer = 0.0
    SimpleCreateEntity("ExplosionParticleEmitter.json")
    SimpleCreateEntity("DashParticleEmitter.json")
    SimpleCreateEntity("HitParticleEmitter.json")
    SimpleCreateEntity("EnemyBulletTrailEmitter.json")
end

function Update()
    if cameraId ~= -1 then
        local cameraTransform = GetTransform(cameraId)
        if cameraShakeTimer > 0.0 then
            cameraShakeTimer = math.max(0.0, cameraShakeTimer - GetDeltaTime())
            local strength = cameraShakeStrength * (cameraShakeTimer / cameraShakeDuration)
            cameraTransform.translate.x = cameraBasePosition.x + (math.random() * 2.0 - 1.0) * strength
            cameraTransform.translate.y = cameraBasePosition.y
            cameraTransform.translate.z = cameraBasePosition.z + (math.random() * 2.0 - 1.0) * strength
        else
            cameraTransform.translate.x = cameraBasePosition.x
            cameraTransform.translate.y = cameraBasePosition.y
            cameraTransform.translate.z = cameraBasePosition.z
        end
    end

    if isGameEnd then-- ゲームが終わった後の処理
        gameEndTimer = gameEndTimer + GetDeltaTime()
        if gameEndTimer >= 1.5 then
            if isNextStage then
                SimpleCreateEntity("ShopManager.json")
                RunAllFunction("OnNextStage")
                isTakeFirstDamagePlayer = false
                isGameEnd = false
                isNextStage = false
                gameEndTimer = 0.0
                stage = stage + 1
                if stage >3 then
                    DebugLog("Finish")
                    LoadScene("ResultScene.json")
                end
            else
                LoadScene("ResultScene.json")
            end
        end
        
    else-- ゲームの終了条件
        
        blockCount = CountEntityTag("block")
        ballCount = CountEntityTag("ball")

        if blockCount <= 0 and CountEntityTag("enemy") <= 0 then
            isGameEnd = true
            isNextStage = true
            DeleteAllTagEntity("enemyBullet")
            DebugLog("Play")
            QFE.Audio.PlaySound(fanfareSE,false,0.2)

            if not isTakeFirstDamagePlayer then
                SimpleCreateEntity("NoDamageUI.json")
            end
        end

        if GetEntityScriptGlobal(beatId,"Pacemaker.lua","bpm") <= 0 then
            isGameEnd = true
            isNextStage = false
            QFE.Audio.PlaySound(missSE,false,0.5)
        end
    end
    

end

function OnPlayerDamage()
    isTakeFirstDamagePlayer = true
    cameraShakeTimer = cameraShakeDuration
end

function OnEnemyKilled()
    cameraShakeTimer = cameraShakeDuration
end

local time = 0.0
local isTransition = false
local isTransitionDebugMode = false
local transitionDuration = 1.5
local sceneName = "GameScene.json"

function Init()
    time = 0.0
    isTransition = false
    -- タイトル再読込後もデバッグモードを維持する
    isTransitionDebugMode = QFE.PostProcess.IsTransitionDebugMode()
    sceneName = "GameScene.json"
end

function Update()
    -- F2 は遷移を開始せず、次回の遷移先をタイトルにするモードを切り替える
    if QFE.Input.GetKeyTrigger("TransitionDebug") then
        isTransitionDebugMode = not isTransitionDebugMode
        QFE.PostProcess.SetTransitionDebugMode(isTransitionDebugMode)
    end

    if not isTransition then
        if QFE.Input.GetKeyTrigger("Jump") then
            if isTransitionDebugMode then
                -- デバッグモード中はタイトルシーン自身へ遷移する
                sceneName = "TitleScene.json"
            else
                sceneName = "GameScene.json"
            end
            isTransition = true
            time = 0.0
            QFE.PostProcess.BeginTransition(isTransitionDebugMode)
        end
    end

    if isTransition then
        time = time + GetDeltaTime()
        local progress = math.min(time / transitionDuration, 1.0)
        QFE.PostProcess.SetTransitionProgress(progress)

        if progress >= 1.0 then
            -- グリッチは遷移先の開始フレームまで維持し、シーン読み込み後にエンジン側で収束させる
            LoadScene(sceneName)
        end
    end
end

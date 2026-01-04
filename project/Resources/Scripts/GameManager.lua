local blockCount = 0
local ballCount = 0
local stage = 0

local isGameEnd = false
local isShop = false
local isNextStage = false

local gameEndTimer = 0.0

local fanfareSE = QFE.Audio.LoadSound("fanfare.wav")
local missSE = QFE.Audio.LoadSound("miss.wav")

function Init()
    blockCount = CountEntityTag("block")
    ballCount = CountEntityTag("ball")
    gameEndTimer = 0.0
end

function Update()
    if isGameEnd then-- ゲームが終わった後の処理
        gameEndTimer = gameEndTimer + GetDeltaTime()
        if gameEndTimer >= 1.5 then
            if isNextStage then
                SimpleCreateEntity("ShopManager.json")
                RunAllFunction("OnNextStage")
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

        if blockCount <= 0 then
            isGameEnd = true
            isNextStage = true
            DebugLog("Play")
            QFE.Audio.PlaySound(fanfareSE,false,0.5)
        end

        if ballCount <= 0 then
            isGameEnd = true
            isNextStage = false
            QFE.Audio.PlaySound(missSE,false,0.5)
        end
    end
    

end

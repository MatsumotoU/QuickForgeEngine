local cardNames ={}
local cardId = {}
local selectId = 1
local oldSelectNum = 1
local tempT = Transform.new()
local leftHintId = -1
local rightHintId = -1
local pulseElapsed = -1.0
local pulseDuration = 0.2
local hintOffsetX = 140.0

local function GetPulseScale()
    if pulseElapsed < 0.0 then
        return 1.0
    end

    local progress = math.min(pulseElapsed / pulseDuration, 1.0)
    return 1.0 + 0.08 * math.sin(progress * math.pi)
end

local function UpdateNavigationHints()
    local selectedTransform = GetTransform(cardId[selectId])
    if selectedTransform ~= nil then
        local selectedX = selectedTransform.translate.x
        local selectedY = selectedTransform.translate.y
        local selectedZ = selectedTransform.translate.z
        SetTranslate(leftHintId, Vector3.new(selectedX - hintOffsetX, selectedY, selectedZ))
        SetTranslate(rightHintId, Vector3.new(selectedX + hintOffsetX, selectedY, selectedZ))
    end

    if leftHintId ~= -1 then
        SetIsDraw(leftHintId, selectId > 1)
        local pulseScale = GetPulseScale()
        SetScale(leftHintId, Vector3.new(pulseScale, pulseScale, pulseScale))
    end
    if rightHintId ~= -1 then
        SetIsDraw(rightHintId, selectId < 3)
        local pulseScale = GetPulseScale()
        SetScale(rightHintId, Vector3.new(pulseScale, pulseScale, pulseScale))
    end
end

function Init()
    selectId = 1
    oldSelectNum = selectId
    pulseElapsed = -1.0
    SimpleCreateEntity("ChooseSkillTitle.json")
    leftHintId = SimpleCreateEntity("ShopLeftHint.json")
    rightHintId = SimpleCreateEntity("ShopRightHint.json")

    -- カード名登録
    cardNames[1] = "BpmCard.json"
    cardNames[2] = "BallCard.json"
    cardNames[3] = "ScoreUpCard.json"
    cardNames[4] = "SpeedCard.json"
    cardNames[5] = "SizeDownCard.json"

    -- 生成
    tempT.translate = transform.translate
    tempT.translate.x = tempT.translate.x-280.0
    tempT.translate.y = tempT.translate.y + 70.0
    for i = 1, 3, 1 do
        cardId[i] = CreateEntity(cardNames[math.random(1,#cardNames)],tempT)
        tempT.translate.x = tempT.translate.x + 280.0
    end

    UpdateScale()
    UpdateNavigationHints()
end

function Update()
    if QFE.Input.GetKeyTrigger("MoveRight") then
        if selectId < 3 then
            selectId = selectId + 1
        end
    end

    if QFE.Input.GetKeyTrigger("MoveLeft") then
        if selectId > 1 then
            selectId = selectId - 1
        end
    end

    local selectionChanged = oldSelectNum ~= selectId
    if selectionChanged then
        pulseElapsed = 0.0
    end

    local shouldUpdateScale = selectionChanged or pulseElapsed >= 0.0
    if pulseElapsed >= 0.0 then
        pulseElapsed = pulseElapsed + GetDeltaTime()
        if pulseElapsed >= pulseDuration then
            pulseElapsed = -1.0
        end
    end

    if shouldUpdateScale then
        UpdateScale()
    end
    UpdateNavigationHints()

    if QFE.Input.GetKeyTrigger("Jump") then
        for i = 1, 3, 1 do
            if i == selectId then
                RunEntityScriptFunction(cardId[i],"Card.lua","Select")
            else
                RunEntityScriptFunction(cardId[i],"Card.lua","NoSelect")
            end
        end
        DeleteAllTagEntity("shopPrompt")
        RunAllFunction("OnShopClosed")
        destroy()
    end

end

function UpdateScale()
    oldSelectNum = selectId
    local pulseScale = GetPulseScale()

    for i = 1, 3, 1 do
         if i == selectId then
            local vec = Vector3.new()
            vec.x = 1.2 * pulseScale
            vec.y = 1.2 * pulseScale
            vec.z = 1.2 * pulseScale
            SetScale(cardId[i],vec)
        else
            local vec = Vector3.new()
            vec.x = 1.0
            vec.y = 1.0
            vec.z = 1.0
            SetScale(cardId[i],vec)
        end
    end
end

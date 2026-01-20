local cardNames ={}
local cardId = {}
local selectId = 1
local oldSelectNum = 1
local tempT = Transform.new()

function Init()
    selectId = 1
    oldSelectNum = selectId
    -- カード名登録
    cardNames[1] = "BpmCard.json"
    cardNames[2] = "BallCard.json"
    cardNames[3] = "ScoreUpCard.json"
    cardNames[4] = "SpeedCard.json"
    cardNames[5] = "SizeDownCard.json"

    -- 生成
    tempT.translate = transform.translate
    tempT.translate.x = tempT.translate.x-220.0
    for i = 1, 3, 1 do
        cardId[i] = CreateEntity(cardNames[math.random(1,#cardNames)],tempT)
        tempT.translate.x = tempT.translate.x + 220.0
    end

    UpdateScale()
end

function Update()
    if QFE.Input.GetKeyTrigger("MoveRight") then
        selectId = selectId +1
        if selectId > 3 then
            selectId = 1
        end
    end

    if QFE.Input.GetKeyTrigger("MoveLeft") then
        selectId = selectId -1
        if selectId < 1 then
            selectId = 3
        end
    end

    if oldSelectNum ~= selectId then
        UpdateScale()
    end

    if QFE.Input.GetKeyTrigger("Jump") then
        for i = 1, 3, 1 do
            if i == selectId then
                RunEntityScriptFunction(cardId[i],"Card.lua","Select")
            else
                RunEntityScriptFunction(cardId[i],"Card.lua","NoSelect")
            end
        end
        destroy()
    end

end

function UpdateScale()
    oldSelectNum = selectId

    for i = 1, 3, 1 do
         if i == selectId then
            local vec = Vector3.new()
            vec.x = 1.2
            vec.y = 1.2
            vec.z = 1.2
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

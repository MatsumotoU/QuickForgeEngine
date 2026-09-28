local timer = 0.0
local isPress = false
local isVisible = true
local waitingForShopClose = false

function Init()
    timer = 0.0
    isPress = false
    isVisible = true
    waitingForShopClose = false
    SetIsDraw(GetThisEntityId(), true)
    SetEntityTag(GetThisEntityId(), "StageStop")
end

function Update()
    if waitingForShopClose or not isVisible then
        return
    end

    local delta = GetDeltaTime()
    timer = timer + delta

    if QFE.Input.GetKeyTrigger("Jump") then
            isPress = true
    end

    if isPress then
        transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,0.0,0.5)
        if transform.scale.x <= 0.0 then
            transform.scale.x = 0.0
            isPress = false
            isVisible = false
            SetIsDraw(GetThisEntityId(), false)
            SetEntityTag(GetThisEntityId(), "Untagged")
        end
    else
        transform.scale.x = 1.0 + math.sin(timer) * 0.1
        transform.scale.y = 1.0 + math.sin(timer) * 0.1
    end

end

function OnNextStage()
    waitingForShopClose = true
end

function OnShopClosed()
    if not waitingForShopClose then
        return
    end

    waitingForShopClose = false
    isPress = false
    isVisible = true
    timer = 0.0
    transform.scale.x = 1.0
    transform.scale.y = 1.0
    transform.scale.z = 1.0
    SetIsDraw(GetThisEntityId(), true)
    SetEntityTag(GetThisEntityId(), "StageStop")
end

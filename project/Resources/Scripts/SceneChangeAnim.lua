reqestClose = false
isEnd = false

local t = 0.0

function Init()
    isEnd = false
    reqestClose = false
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function Update()
    if not reqestClose then
        return
    end

    if t < 1.0 then
        t = t + GetDeltaTime()*3.0
    else
        t = 1.0
        isEnd = true
    end

    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = t
end

function ReqestClose()
    reqestClose = true
end
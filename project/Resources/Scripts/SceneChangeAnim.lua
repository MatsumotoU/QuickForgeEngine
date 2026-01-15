reqestClose = false
isEnd = false

local t = 0.0

function Init()
    isEnd = false
    reqestClose = false
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 1.0
end

function Update()
    local mat = GetMaterial(GetThisEntityId())
    if not reqestClose then
        mat.color.w = QFE.Math.SimpleEaseIn(mat.color.w,0.0,0.1)
        return
    end

    if t < 1.0 then
        t = t + GetDeltaTime()*3.0
    else
        t = 1.0
        isEnd = true
    end
    mat.color.w = t
end

function ReqestClose()
    reqestClose = true
end
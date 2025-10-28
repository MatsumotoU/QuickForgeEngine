function Init()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 1.0
end

function Update()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = QFE.Math.SimpleEaseIn(mat.color.w,0.0,0.5)

    if mat.color.w <= 0.1 then
        destroy()
    end
end

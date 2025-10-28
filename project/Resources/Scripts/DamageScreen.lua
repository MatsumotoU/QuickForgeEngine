function Init()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function Update()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = QFE.Math.SimpleEaseIn(mat.color.w,0.0,0.05)
end

function Damage()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.7
end

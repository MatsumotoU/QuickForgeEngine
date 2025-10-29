isDraw = true

function Init()
    local mat = GetMaterial(this.GetEntityId())
    mat.color.w = 1.0
end

function Update()
    local mat = GetMaterial(this.GetEntityId())
    if isDraw then
        mat.color.w = QFE.Math.SimpleEaseIn(mat.color.w,1.0,0.3)
    else
        mat.color.w = QFE.Math.SimpleEaseIn(mat.color.w,0.0,0.7)
    end
end

function OnDraw()
    isDraw = true
end

function OffDraw()
    isDraw = false
end
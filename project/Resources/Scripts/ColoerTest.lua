local mat = Material.new()

function Init()
    mat = GetMaterial(GetThisEntityId())
    mat.color.x = 0.5
    DebugLog("Init")
end

function Update()
    
end

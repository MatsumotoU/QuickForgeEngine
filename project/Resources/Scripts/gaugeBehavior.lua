
-- マテリアル
local mat = Material.new()

function Init()
     -- マテリアル情報を取得
    mat = GetMaterial(GetThisEntityId())
    mat.color.x = 1.0
    mat.color.y = 0.0
    mat.color.z = 0.0
end

function Update()

end

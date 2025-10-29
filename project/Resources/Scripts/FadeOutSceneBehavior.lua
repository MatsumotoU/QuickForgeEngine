
local timer = 0.0
local maxTime = 1.0

-- マテリアル
local mat = Material.new()

function Init()
    timer = 0.0
    -- マテリアル情報を取得
    mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function Update()

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / maxTime,1.0)

    mat.color.w = QFE.Math.Leap(1.0,0.0,t)

    if timer >= maxTime then
         -- タイトルシーンに移動
        LoadScene("TitleScene")
    end

end

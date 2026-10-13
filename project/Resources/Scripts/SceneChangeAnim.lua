-- 旧来の白フェード用エンティティとの互換スクリプトです。
-- シーン遷移の見た目は PostProcess のグリッチへ移行しています。

function Init()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function Update()
    local mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

function ReqestClose()
    -- 互換用。実際の遷移開始は QFE.PostProcess.BeginTransition で行います。
end

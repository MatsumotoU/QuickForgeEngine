
LockOnObjName = "objName"

local id = 0

-- 速度
speed = 0.2

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

function Init()
id = GetEntity(LockOnObjName)
end

function Update()
    -- 目的の位置を取得
    targetTransform = GetTransform(id)
    targetTransform.translate.y = 0.0
    dir = targetTransform.translate - transform.translate

    -- 距離を求める
    local distance = dir:Length()

    if distance > 2.0 then
        -- 正規化する
        dir:Normalize()
        -- 移動量を取得
        move = dir * speed
        -- 移動
        transform.translate = transform.translate + move
    else
        -- 移動
        transform.translate = transform.translate + move
    end

end

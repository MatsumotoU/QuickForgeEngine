-- ロックオンするオブジェクトの名前
LockOnObjName = "objName"

local id = 0

-- 移動量
move = Vector3.new(0.0,0.0,0.0)

-- ヒットフラグ
local isHit = false

-- 弾のオブジェクト
bulletName = "EnemyBullet.json"

-- 弾を発射する時間(秒)
bulletShotTime = 1.0
-- 弾の発射を計測する
timer = 0.0

--[[
    初期化処理
--]]
function Init()
id = GetEntity(LockOnObjName)
DebugLog("LockOnEnemyTargetID: "..id)
end

--[[
    更新処理
--]]
function Update()

    timer = timer + (1.0 / (60.0 * bulletShotTime))
    
    id = GetEntity(LockOnObjName)
    -- 目的の位置を取得
    local targetTransform = GetTransform(id)
    targetTransform.translate.y = 0.0
    -- ベクトルを取得
    local dir = Vector3.new(0.0,0.0,0.0)
    dir.x = targetTransform.translate.x - transform.translate.x
    dir.y = 0.0
    dir.z = targetTransform.translate.z - transform.translate.z
    -- 正規化する
    dir:Normalize()
    -- ターゲットの方向に向ける
    local angleY = math.atan(dir.x,dir.z)
    transform.rotate.y = angleY

    if timer >= 1.0 then
        local tmp = Transform.new()
        tmp.translate.x = transform.translate.x + dir.x * 0.3
        tmp.translate.z = transform.translate.z + dir.z * 0.3
        tmp.rotate.y = transform.rotate.y
        -- 弾を生成
        CreateEntity(bulletName,tmp)
        timer = 0.0
    end
end

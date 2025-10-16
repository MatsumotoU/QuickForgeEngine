
-- 壊れているかを判断する
local isBreak = false

-- 墓石に当たったノックバック回数
local nockbackCount = 0
-- ノックバックによって墓石が壊れる回数
maxBreakNockbackCount = 2

-- プレイヤーが触れていると壊れるカウント
local breakTimer = 0.0
-- 壊れる時間(秒)
breakMaxTime = 1.0

-- スローエリアのオブジェクト
slowObjName = "obj"

--[[
    初期化処理
--]]
function Init()
    isBreak = false
end

--[[
    更新処理
--]]
function Update()

end

function OnCollisionEnter(id,obj)

    if isBreak then
        -- スローエリアを生成
        CreateEntity(slowObjName,transform)
        destroy()
    else
        if obj.tag == "Player" then
            -- プレイヤーの位置を取得
            local playerTransform = GetTransform(id)

            -- プレイヤーがブロックに触れている時
            if transform.translate.x >= playerTransform.translate.x then
                breakTimer = breakTimer + 1.0 / (60.0 * breakMaxTime)
                -- 時間がたったら壊れる
                if breakTimer >= 1.0 then
                    isBreak = true
                end

            end         
        end
    end
end

function OnCollisionStay(id,obj)
    -- 墓石が壊れていれば早期リターン
    if isBreak then
        return
    end

    -- 弾を打たれた時
    if obj.tag == "Bullet" then
        isBreak = true
    end

    -- ノックバック攻撃を食らった時
    if obj.tag == "Player" then
        -- プレイヤーの位置を取得
        local playerTransform = GetTransform(id)

        if transform.translate.x <= playerTransform.translate.x then
            nockbackCount = nockbackCount + 1
            -- ノックバック回数
            if nockbackCount >= maxBreakNockbackCount then
                isBreak = true
            end
        end         
    end
end
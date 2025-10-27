
-- 壊れているかを判断する
local isBreak = false

-- 墓石に当たったノックバック回数
nockbackCount = 0
-- ノックバックによって墓石が壊れる回数
maxBreakNockbackCount = 2

-- プレイヤーが触れていると壊れるカウント
local breakTimer = 0.0
-- 壊れる時間(秒)
breakMaxTime = 1.0

-- スローエリアのオブジェクト
slowObjName = "obj"

-- 弾薬の増える数
dropBullets = 2

playerName = "ShotGunPlayer"

local timer = 0.0
local maxTime = 1.0

-- 音
local breakSE = QFE.Audio.LoadSound("tomBreak.mp3")

--[[
    初期化処理
--]]
function Init()
    isBreak = false
    nockbackCount = 0
    force.velocity.y = -5.0
end

--[[
    更新処理
--]]
function Update()

    -- 出現してから当たり判定が適応されるまでの時間
    if timer <= 1.0 then
        local deltatime = GetDeltaTime()
        timer = timer + deltatime
    end

    if maxBreakNockbackCount > 0 then
        transform.scale.y = (maxBreakNockbackCount - nockbackCount) / maxBreakNockbackCount
    end
end

function OnCollisionEnter(id,obj)

    if isBreak then
        -- スローエリアを生成
        CreateEntity(slowObjName,transform)
        
        destroy()
    else
        --if obj.tag == "player" then
            -- プレイヤーの位置を取得
            --local playerTransform = GetTransform(id)

            -- プレイヤーがブロックに触れている時
            --if transform.translate.x >= playerTransform.translate.x then
                --breakTimer = breakTimer + 1.0 / (60.0 * breakMaxTime)
                --transform.rotate.y = transform.rotate.y + breakTimer
                -- 時間がたったら壊れる
                --if breakTimer >= 1.0 then
                --    isBreak = true
              --  end
           -- end         
        --end
    end
end

function OnCollisionStay(id,obj)
    -- 墓石が壊れていれば早期リターン
    if isBreak then
        return
    end

    -- 弾を打たれた時
    if obj.tag == "bullet" then
        if timer >= 1.0 then
            isBreak = true
            for i = 1, dropBullets, 1 do
            RunEntityScriptFunction(GetEntity(playerName),"BulletShot.lua","ReloadOne")
            end
            RunEntityScriptFunction(GetEntity("ShallReload"),"ShallReloadUI.lua","Anim")
            CreateEntity("TombstoneEmitter.json",transform)
            QFE.Audio.PlaySound(breakSE,false,0.3)
        end
    end

    -- ノックバック攻撃を食らった時
    if obj.tag == "player" then
        -- プレイヤーがノックバックしているなら
        local isKB = GetEntityScriptGlobal(id,"BulletShot.lua","isKnockback")
        if not isKB then
            return
        end
        
        -- ノックバックできる回数減少
        nockbackCount = nockbackCount + 1
        -- ノックバック回数
        if nockbackCount >= maxBreakNockbackCount then
            isBreak = true
            for i = 1, dropBullets, 1 do
            RunEntityScriptFunction(GetEntity(playerName),"BulletShot.lua","ReloadOne")
            end
            RunEntityScriptFunction(GetEntity("ShallReload"),"ShallReloadUI.lua","Anim")
            QFE.Audio.PlaySound(breakSE,false,0.3)
        end       
    end
end
maxBlockHp = 3
blockHp = maxBlockHp

local t = 1.0

function Init()
    blockHp = maxBlockHp
    t = 1.0
end

function Update()
    if t <= 1.0 then
        t = t + GetDeltaTime() * 2.0
    end

    if t > 1.0 then
        t = 1.0
    end

    transform.scale.x = QFE.Math.EaseIn(transform.scale.x,1.0,t);
    transform.scale.z = QFE.Math.EaseIn(transform.scale.z,1.0,t);
end

function OnCollisionStay(id,obj)
    -- 弾を打たれた時
    if obj.tag == "bullet" then
        destroy()
    end

    -- ノックバック攻撃を食らった時
    if obj.tag == "player" then
        -- プレイヤーがノックバックしているなら
        local isKB = GetEntityScriptGlobal(id,"BulletShot.lua","isKnockback")
        if not isKB then
            return
        end
        
        t = 0.0
        transform.scale.x = 0.5
        transform.scale.z = 0.5

        -- ノックバックできる回数減少
        blockHp = blockHp - 1
        -- ノックバック回数
        if blockHp <= 0 then
            destroy()
        end       
    end
end
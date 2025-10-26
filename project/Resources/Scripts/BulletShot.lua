bulletName = "Bullet.json"
shotNum = 3
diffusionRate = 2.0

knockBackPower = 3.0

shotInterval = 0.0
maxShotInterval = 1.0

bullets = 1
maxBullets = 3

reloadInterval = 0.0
targetReloadInterval = 0.3

isKnockback = false

local deltatime = 0.016

-- リロードしたかを取得
isReload = false
-- 打ったかを取得
isShot = false


function Init()
shotInterval = 0.0
isKnockback = false
end

function Update()

    isReload = false
    isShot = false

    deltatime = GetDeltaTime()

    -- ノックバックしているか判断
    if isKnockback then
        if force.velocity:Length() <= 0.0 then
            isKnockback = false
        end
    end

    -- リロード
    if QFE.Input.GetKeyPress("MoveLeft") then
        if force.velocity:Length() > 0.3 then
            return
        end

        isReload = true
        if bullets == 0 then
            reloadInterval = reloadInterval + deltatime*2.0
        else
            reloadInterval = reloadInterval + deltatime
        end

        if reloadInterval >= targetReloadInterval then
        reloadInterval = 0.0
        bullets = bullets + 1
        if bullets <= maxBullets then
            RunEntityScriptFunction(GetEntity("ShotGun"),"ShotGunAnim.lua","Reroad")
        end        

            if bullets >= maxBullets then
                bullets = maxBullets
            end
        end

    else
        reloadInterval = 0.0
    end

    -- 射撃のクールダウンが終わってなければここで関数終了
    if shotInterval > 0.0 then
        if force.velocity:Length() > 1.0 then
            return
        end
        shotInterval = shotInterval - deltatime
        RunEntityScriptFunction(GetEntity("ShotGun"),"ShotGunAnim.lua","PumpAction")
        return
    end

    -- 弾倉管理
    if bullets <= 0 then
        return
    end

    -- 射撃処理
    if QFE.Input.GetKeyTrigger("Shot") then
        local tempTransform = Transform.new()
        tempTransform.translate = transform.translate
        tempTransform.rotate = transform.rotate

        
        isShot = true

        for i = 1, shotNum, 1 do

            local tmp = i % 2
            local max_addNum = math.pi * diffusionRate
            local addNum = 0

            if tmp == 0 then
                  addNum =  max_addNum * math.random() 
            else
                addNum  = -max_addNum * math.random()
            end


            tempTransform.rotate.y = transform.rotate.y + addNum
            CreateEntity(bulletName,tempTransform)
        end

        force.velocity.x = -math.sin(transform.rotate.y) * knockBackPower
        force.velocity.z = -math.cos(transform.rotate.y) * knockBackPower

        shotInterval = maxShotInterval
        bullets = bullets - 1
        isKnockback = true
        transform.rotate.x = -1.0
        RunEntityScriptFunction(GetEntity("ShotGun"),"ShotGunAnim.lua","Shot")
    end
end

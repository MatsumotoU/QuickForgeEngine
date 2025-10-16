bulletName = "Bullet.json"
shotNum = 3
diffusionRate = 2.0

knockBackPower = 3.0

shotInterval = 0.0
maxShotInterval = 1.0

bullets = 1
maxBullets = 3

isKnockback = false

function Init()
shotInterval = 0.0
isKnockback = false
end

function Update()
    -- ノックバックしているか判断
    if isKnockback then
        if force.velocity:Length() <= 0.0 then
            isKnockback = false
        end
    end

    -- リロード
    if QFE.Input.GetKeyTrigger("MoveLeft") then
        bullets = maxBullets
    end

    -- 射撃のクールダウンが終わってなければここで関数終了
    if shotInterval > 0.0 then
        if force.velocity:Length() > 1.0 then
            return
        end
        shotInterval = shotInterval - 0.016
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

        for i = 1, shotNum, 1 do
            tempTransform.rotate.y = transform.rotate.y + (math.random() * diffusionRate - (diffusionRate * 0.5))
            CreateEntity(bulletName,tempTransform)
        end

        force.velocity.x = -math.sin(transform.rotate.y) * knockBackPower
        force.velocity.z = -math.cos(transform.rotate.y) * knockBackPower

        shotInterval = maxShotInterval
        bullets = bullets - 1
        isKnockback = true
    end
end

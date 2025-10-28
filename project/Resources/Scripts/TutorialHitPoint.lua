
borderName = "TutorialDamageBorder"

function Init()
    borderId = GetEntity(borderName)
end

function Update()

    -- ボーダーダメージ
    if transform.translate.x < GetTransform(borderId).translate.x then
        --hitPoint = hitPoint - 1
        --do_reload = true
        --isDamaged = true
        force.velocity.x = force.velocity.x + 92.5
        --force.velocity.y = force.velocity.y + 20
        --damageInterval = maxDamageInterval
        CreateEntity("ExplotionParticleEmitter.json",transform)
    end

end

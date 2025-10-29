
borderName = "TutorialDamageBorder"

TutorialManagerJson = "TutorialManager"
local ID = 0
local TutorialScriptName = "TutorialManager.lua"
local eventTypeVarName = "EventType"

function Init()
    borderId = GetEntity(borderName)

    -- チュートリアルIDを取得
    ID = GetEntity(TutorialManagerJson)
end

function Update()

    local eType = GetEntityScriptGlobal(ID,TutorialScriptName,eventTypeVarName)

    if eType == 6 then
        -- ボーダーダメージ
    if transform.translate.x < GetTransform(borderId).translate.x then
        RunEntityScriptFunction(ID,TutorialScriptName,"AddNum")
        --hitPoint = hitPoint - 1
        --do_reload = true
        --isDamaged = true
        force.velocity.x = force.velocity.x + 92.5
        --force.velocity.y = force.velocity.y + 20
        --damageInterval = maxDamageInterval
        CreateEntity("ExplotionParticleEmitter.json",transform)
    end

    end

end

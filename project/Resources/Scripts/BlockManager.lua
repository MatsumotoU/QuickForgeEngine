local timer = 0.0
local stage = 1

function Init()
    OnNextStage()
end

function Update()
    local c = CountEntityTag("block")
    if c <= 0 then
        local delta = GetDeltaTime()
        timer = timer + delta
        if timer > 3.0 then
            --RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            --LoadScene("TitleScene.json")
        end
    end
end

function OnNextStage()
    if stage == 1 then
        CreateStage1()
    elseif stage == 2 then
        CreateStage2()
    elseif stage == 3 then
        CreateStage3()
    elseif stage == 4 then
        CreateStage4()
    elseif stage == 5 then
        CreateStage5()
    else
        CreateStage6()
    end
    stage = stage+ 1
end

function CreateStage1()
    local temp = Transform.new()
    temp.translate.x = -1.8 * 2.0
    temp.scale.x = 1.5
    temp.scale.y = 0.5
    temp.scale.z = 0.8
    
    for y = 0,3 do
        for i = 0,4 do
            if y ==2 and i %2==0 and i ~= 2 then
                CreateEntity("LockOnEnemy.json",temp)
            end
            
            temp.translate.x =temp.translate.x +1.8
        end
        temp.translate.x = -1.8 * 2.0
        temp.translate.z =temp.translate.z +1.2
    end
end

function CreateStage2()
    local temp = Transform.new()
    temp.translate.x = -1.8 * 2.0
    temp.scale.x = 1.5
    temp.scale.y = 0.5
    temp.scale.z = 0.8
    
    for y = 0,3 do
        for i = 0,4 do
            if y ==2 and i %2==0 then
                if i == 2 then
                    CreateEntity("WayEnemy.json",temp)
                else
                    CreateEntity("LockOnEnemy.json",temp)
                end
            end
            
            temp.translate.x =temp.translate.x +1.8
        end
        temp.translate.x = -1.8 * 2.0
        temp.translate.z =temp.translate.z +1.2
    end
end

function CreateStage3()
    local temp = Transform.new()
    temp.translate.x = -1.8 * 2.0
    temp.scale.x = 1.5
    temp.scale.y = 0.5
    temp.scale.z = 0.8
    
    for y = 0,3 do
        for i = 0,4 do
            if y %2==0 and i %2==0 then
                if i == 2 and y == 0 then
                    CreateEntity("ClusterEnemy.json",temp)
                else
                    CreateEntity("LockOnEnemy.json",temp)
                end
            end
            
            temp.translate.x =temp.translate.x +1.8
        end
        temp.translate.x = -1.8 * 2.0
        temp.translate.z =temp.translate.z +1.2
    end
end

function CreateStage4()
    SimpleCreateEntity("BossEnemy.json")
end

function CreateStage5()
    local temp = Transform.new()
    temp.translate.x = 0.0
    temp.translate.y = 0.0
    temp.scale.x = 1.0
    temp.scale.y = 1.0
    temp.scale.z = 1.0

    -- Spawn 2 Cluster Enemies
    CreateEntity("ClusterEnemy.json", temp)
    
    temp.translate.x = -3.0
    CreateEntity("ClusterEnemy.json", temp)

    temp.translate.x = 3.0
    CreateEntity("ClusterEnemy.json", temp)
end

function CreateStage6()
    local temp = Transform.new()
    temp.translate.x = 0.0
    temp.translate.y = 0.0
    temp.scale.x = 1.0
    temp.scale.y = 1.0
    temp.scale.z = 1.0

    -- Spawn 2 Curve Enemies
    CreateEntity("CurveEnemy.json", temp)
    
    temp.translate.x = -3.0
    CreateEntity("CurveEnemy.json", temp)

    temp.translate.x = 3.0
    CreateEntity("CurveEnemy.json", temp)
end
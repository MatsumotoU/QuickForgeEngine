local timer = 0.0

function Init()
    local temp = Transform.new()
    temp.translate.x = -1.8 * 2.0
    temp.scale.x = 1.5
    temp.scale.y = 0.5
    temp.scale.z = 0.8
    
    for y = 0,3 do
        for i = 0,4 do
            if y ==2 and i == 2 then
                CreateEntity("EchoBlock.json",temp)
            else
                CreateEntity("Block.json",temp)
            end
            
            temp.translate.x =temp.translate.x +1.8
        end
        temp.translate.x = -1.8 * 2.0
        temp.translate.z =temp.translate.z +1.2
    end
    
end

function Update()
    

    local c = CountEntityTag("block")
    if c <= 0 then
        local delta = GetDeltaTime()
        timer = timer + delta
        --Echo(transform.translate,8.0)
        if timer > 3.0 then
            --RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            --LoadScene("TitleScene.json")
        end
    end
end

function OnNextStage()
    Init()
end

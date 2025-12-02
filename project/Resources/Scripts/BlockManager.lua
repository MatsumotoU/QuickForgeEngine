local timer = 0.0

function Init()
    local temp = Transform.new()
    temp.translate.x = -4.4
    temp.scale.y = 0.5
    temp.scale.z = 0.5
    
    for y = 0,4 do
        for i = 0,8 do
            if i == 4 and y == 2 then
                CreateEntity("EchoBlock.json",temp)
            else
                CreateEntity("Block.json",temp)
            end
        temp.translate.x =temp.translate.x +1.1
        end
        temp.translate.x = -4.4
        temp.translate.z =temp.translate.z +1.2
    end
    
end

function Update()
    

    local c = CountEntityTag("block")
    if c <= 0 then
        local delta = GetDeltaTime()
        timer = timer + delta
        Echo(transform.translate,8.0)
        if timer > 3.0 then
            RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
            LoadScene("TitleScene.json")
        end
    end
end

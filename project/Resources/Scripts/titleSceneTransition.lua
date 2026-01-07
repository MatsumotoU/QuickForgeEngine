local time = 3.0
local isTransition = false
sceneName = "GameScene.json"

function Init()
time = 3.0
end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
        isTransition = true;
        RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
    end

    if isTransition then
        if time < 0.0 then
            LoadScene(sceneName)   
        else 
            time = time - GetDeltaTime()
        end
    end
end

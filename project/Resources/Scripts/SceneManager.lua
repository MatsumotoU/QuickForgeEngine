nextSceneName = "nextScene"
local isTransit = false

function Init()
    isTransit = false
end

function Update()
    local isEnd = GetEntityScriptGlobal(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","isEnd")
    if isEnd and isTransit then
        LoadScene(nextSceneName)
    end
end

function OnCollisionStay(id,obj)
    if obj.tag == "player" then
        if not isTransit then
            isTransit = true
            RunEntityScriptFunction(GetEntity("SceneChangeAnim"),"SceneChangeAnim.lua","ReqestClose")
        end
    end
end
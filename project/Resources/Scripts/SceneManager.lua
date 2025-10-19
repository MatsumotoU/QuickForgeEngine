nextSceneName = "nextScene"
local isTransit = false

function Init()
    isTransit = false
end

function Update()

end

function OnCollisionStay(id,obj)
    if obj.tag == "player" then
        if not isTransit then
            isTransit = true
            LoadScene(nextSceneName)
        end
    end
end
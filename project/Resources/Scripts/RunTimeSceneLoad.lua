sceneName = "name"

function Init()

end

function Update()
    if QFE.Input.GetKeyTrigger("Decide") then
        LoadScene(sceneName)
    end
end

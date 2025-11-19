local time = 3.0
local isTransition = false

function Init()
time = 3.0
end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
        isTransition = true;
    end

    if isTransition then
        if time < 0.0 then
            LoadScene("GameScene.json")   
        else 
            time = time - GetDeltaTime()
        end
    end
end

dig = 3
num = 0
local nums ={}
local tempT = Transform.new()
function Init()
    
    tempT.translate = transform.translate
    tempT.rotate.x = 1.57
    tempT.rotate.z = 3.14
 
    for i = 1, dig, 1 do
        nums[i] = CreateEntity("Number.json",tempT)
        tempT.translate.x = tempT.translate.x + 0.5
    end
end

function Update()
    -- tempT.translate = transform.translate
    -- tempT.rotate.x = 1.57
    -- tempT.rotate.z = 3.14

    local temp = num
    for i = 1, dig, 1 do
        -- SetTransform(nums[i],tempT)
        -- tempT.translate.x = tempT.translate.x + 0.5
        SetEntityScriptGlobal(nums[dig - i + 1],"Number.lua","num",math.floor(temp % 10)) 
        temp = temp / 10
    end
end

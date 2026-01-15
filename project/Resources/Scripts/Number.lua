num = 0
local oldNum = num
local meshtable = {}


function Init()
    oldNum = num
    meshtable[1] = "0.obj"
    meshtable[2] = "1.obj"
    meshtable[3] = "2.obj"
    meshtable[4] = "3.obj"
    meshtable[5] = "4.obj"
    meshtable[6] = "5.obj"
    meshtable[7] = "6.obj"
    meshtable[8] = "7.obj"
    meshtable[9] = "8.obj"
    meshtable[10] = "9.obj"

    ChangeMesh(this.GetEntityId(),meshtable[num + 1])
end

function Update()
    if num <= 0 then
        num = 0
    end
    if num >= 9 then
        num = 9
    end

    if oldNum ~= num then
        oldNum = num
        ChangeMesh(this.GetEntityId(),meshtable[num + 1])
    end
end

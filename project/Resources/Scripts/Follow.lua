followObj = "PlayerBar"
local followId = 0

function Init()
    followId = GetEntity(followObj)
end

function Update()
    transform.translate = GetTransform(followId).translate
end

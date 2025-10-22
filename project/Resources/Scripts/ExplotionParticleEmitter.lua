particleObjName = "RedDotParticle.json"
ids ={}
function Init()
    for i = 1, 10, 1 do
        tempTransform = Transform.new()
        tempTransform.scale = transform.scale
        tempTransform.translate = transform.translate
        tempTransform.rotate.x = math.random() * 6.28
        tempTransform.rotate.y = math.random() * 6.28
        tempTransform.rotate.z = math.random() * 6.28
        CreateEntity(particleObjName,tempTransform)
    end
end

function Update()
    destroy()
end
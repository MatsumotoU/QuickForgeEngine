particleObjName = "RedDotParticle.json"
ids ={}
function Init()
    for i = 1, 10, 1 do
        tempTransform = Transform.new()
        tempTransform.scale.x = 0.5
        tempTransform.scale.y = 0.5
        tempTransform.scale.z = 0.5
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
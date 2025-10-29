local playerId = 0
stayTime = 0.0

function Init()
    playerId = GetEntity("ShotGunPlayer")
end

function Update()
    local length = Vector3.new()
    length = transform.translate - GetTransform(playerId).translate

    local len = length:Length()
    if len >= 1.5 then
        transform.translate.x = QFE.Math.SimpleEaseIn(transform.translate.x,transform.translate.x - length:Normalize().x * GetDeltaTime(),1.0) 
        transform.translate.y = QFE.Math.SimpleEaseIn(transform.translate.y,transform.translate.y - length:Normalize().y * GetDeltaTime(),1.0) 
        transform.translate.z = QFE.Math.SimpleEaseIn(transform.translate.z,transform.translate.z - length:Normalize().z * GetDeltaTime(),1.0) 
        stayTime = 0.0
        return
    end
    stayTime = stayTime + GetDeltaTime()

    if stayTime > 5.0 then
        local rm = math.random(0,2)
        local temp = Transform.new()
        temp.translate = transform.translate
        temp.translate.y = 0.02
        temp.rotate.y = 3.14
        temp.translate.x = temp.translate.x + 2.5
        temp.translate.z = temp.translate.z + 1.5
        stayTime = 0.0
        if rm == 0 then
            CreateEntity("Message1.json",temp)
        elseif rm == 1 then
            CreateEntity("Message2.json",temp)
        elseif rm == 2 then
            CreateEntity("Message3.json",temp)
        end 
    end
end

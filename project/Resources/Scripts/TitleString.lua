local spinSpeed = 0.0

function Init()
    spinSpeed = 0.0
end

function Update()
    if math.abs(spinSpeed) > 0.0 then
        spinSpeed = spinSpeed * 0.98
    end

    if  math.abs(spinSpeed) <= 0.1 then
        spinSpeed = 0.0
        transform.rotate.y = QFE.Math.SimpleEaseIn(transform.rotate.y,3.14,0.1)
    end

    transform.rotate.y = transform.rotate.y + spinSpeed * GetDeltaTime()
end

function OnCollisionEnter(id,obj)
    if spinSpeed > 0.0 then
        return
    end

    local power = 2.0
    if obj.tag == "bullet" then
        power = 30.0
    end

    local tempTransform = Transform.new()
    tempTransform = GetTransform(id)
    local spinX = (transform.translate.x - tempTransform.translate.x)
    local spinY = (transform.translate.z - tempTransform.translate.z)
    spinSpeed = (spinX * spinY) * power
    
end
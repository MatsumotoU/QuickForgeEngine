
speed = 0.0

function Init()
DebugLog("Init")
end

function Update()

-- 移動
transform.translate.x = transform.translate.x - speed * (1.0 / 60.0)
end

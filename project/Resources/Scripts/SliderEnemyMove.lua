
-- 移動する幅を設定
minMove = 0.0
maxMove = 0.0

-- 移動する時間
moveTime = 0.0
-- スローの移動する時間
local slowMoveTime = 0.0

local moveMaxTime = 0.0

local timer = 0.0

-- 1つのインスタンスが持つ移動幅
local move = 0.0

-- ヒットしているかのフラグ
local isHit = false

function Init()
DebugLog("Init")
DebugLog(minMove)
DebugLog(maxMove)
-- 移動量を取得
move = RandomRange(minMove,maxMove)
-- スロー時間を求める
slowMoveTime = slowMoveTime + moveTime * 2.0
moveMaxTime = moveTime
end

function Update()

    if isHit then
        moveMaxTime = slowMoveTime
    else
        moveMaxTime = moveTime
    end

    timer = timer + 1.0 / (60.0 * moveMaxTime)

    if timer <= 0.5 then
        local moveLeftTimer = timer / 0.5
        transform.translate.z = transform.translate.z + QFE.Math.Leap(-move,move,moveLeftTimer)
    else
        local moveRightTimer = (timer - 0.5) / 0.5
        transform.translate.z = transform.translate.z + QFE.Math.Leap(move,-move,moveRightTimer)
    end

    if timer >= 1.0 then
        timer = 0.0
    end

    isHit = false

end

function RandomRange(min, max)
    return min + math.random() * (max - min)
end

function OnCollisionEnter(id,obj)

    if obj.tag == "SlowArea" then
        isHit = true
    end
end
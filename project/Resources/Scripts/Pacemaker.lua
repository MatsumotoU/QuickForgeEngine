bpm = 60.0
nume = 4
dino = 4
local time = 0.0          -- 経過時間カウンター
local seconds_per_beat = 0.0 -- 1拍の長さ（秒）
local seconds_per_weakbeat = 0.0
local oldBpm = bpm
local currentBeat = 0

function Init()
    nume = 4
    dino = 4
    oldBpm = bpm
    -- 1拍の長さを計算 (60秒 / BPM)
    seconds_per_beat = 60.0 / bpm
    currentBeat = 0
end

function Update()
    time = time + GetDeltaTime()

    -- 1拍の長さを計算 (60秒 / BPM)
    seconds_per_beat = 60.0 / bpm

    -- 拍の長さ（seconds_per_beat）と比較する
    if time >= seconds_per_beat then
        time = time - seconds_per_beat

        -- 一拍ずつの関数実行
        currentBeat = currentBeat +1
        if currentBeat >= 4 then
            RunAllFunction("OnBar")
            transform.scale.x = 1.5
            transform.scale.z = 2.0
            currentBeat = 0
        else
            RunAllFunction("OnStrongBeat")
            transform.scale.x = 1.5
            transform.scale.z = 1.3
        end
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,1.0,0.1)

    if oldBpm ~= bpm then
        oldBpm = bpm
        RunAllFunction("OnPlayerChangeBpm")
    end
end

function OnUpGradeBpmCard()
    bpm = bpm + 30
end
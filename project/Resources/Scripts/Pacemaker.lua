bpm = 60.0
nume = 4
dino = 4
local time = 0.0          -- 経過時間カウンター
local seconds_per_beat = 0.0 -- 1拍の長さ（秒）
local seconds_per_weakbeat = 0.0

local strongBeatSE = QFE.Audio.LoadSound("StrongBeat.wav")

function Init()
    bpm = 60.0
    nume = 4
    dino = 4
    -- 1拍の長さを計算 (60秒 / BPM)
    seconds_per_beat = 60.0 / bpm
end

function Update()
    time = time + GetDeltaTime()

    -- 1拍の長さを計算 (60秒 / BPM)
    seconds_per_beat = 60.0 / bpm

    -- 拍の長さ（seconds_per_beat）と比較する
    if time >= seconds_per_beat then
        time = time - seconds_per_beat

        -- 一拍ずつの関数実行
        RunAllFunction("OnStrongBeat")
        transform.scale.x = 2.0
        transform.scale.z = 2.0
        --QFE.Audio.PlaySound(strongBeatSE,false,1.0)
    end

    transform.scale.x = QFE.Math.SimpleEaseIn(transform.scale.x,1.0,0.1)
    transform.scale.z = QFE.Math.SimpleEaseIn(transform.scale.z,1.0,0.1)
end
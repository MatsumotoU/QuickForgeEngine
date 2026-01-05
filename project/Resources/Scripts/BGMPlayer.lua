bgmName = "bgm.mp3"
vol = 0.3
local bgm = 0

function Init()
    bgm = QFE.Audio.LoadSound(bgmName)
    QFE.Audio.PlaySound(bgm,true,vol)
end

function Update()

end

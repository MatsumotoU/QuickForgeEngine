bgmName = "BGM"
volume = 0.3

function Init()
    QFE.Audio.PlaySound(QFE.Audio.LoadSound(bgmName),true,volume)
end

function Update()

end


-- シーン切り替えに使用する時間(秒)
fadeInTime = 0.5
waitTime = 0.4
fadeOutTime = 0.5

-- タイマー
local timer = 0.0

-- シーンの実行タイプ(0:フェードイン, 1:待ち, 2:フェードアウト)
local runType = 0

-- クリアされたかを取得する名前
mapManagerObjName = "MapManager"
mapManagerScriptName = "MapManager.lua"
varflagName = "isClear"
varIsDeadName = "isDead"
local linkID = 0
local isClear = false

-- リセットするためのフラグ
isReset = false

-- 実行中かを判断するフラグ
isRun = false

-- 死亡判定を取得する
isDead = false

-- マテリアル
local mat = Material.new()

-- シーン遷移する間隔を調整する
local intervalTime = 3.0
local intervalTimer = 0.0
local isPlay = false

-- カメラの位置だけを先に移動させる処理
isPreReset = false
local fremeCount = 0

local hasSentReset = false

--[[
    初期化処理
--]]
function Init()
    isReset = false
    isPreReset = false
    fremeCount = 0
    -- クリアしたかを取得する
    linkID = GetEntity(mapManagerObjName)
    DebugLog("LinkedID:"..linkID)
    DebugLog(mapManagerScriptName)
    DebugLog(varflagName)
    isClear = GetEntityScriptGlobal(linkID,mapManagerScriptName,varflagName)
    -- マテリアル情報を取得
    mat = GetMaterial(GetThisEntityId())
    mat.color.w = 0.0
end

--[[
    更新処理
--]]
function Update()

    if intervalTimer < intervalTime + 1.0 then
    local deltatime = GetDeltaTime()
    intervalTimer = intervalTimer + deltatime
    end

    if intervalTimer >= intervalTime then
        isPlay = true
    else
        isPlay = false
    end

    isClear = GetEntityScriptGlobal(linkID,mapManagerScriptName,varflagName)
    local tmpIsDead = GetEntityScriptGlobal(linkID,mapManagerScriptName,varIsDeadName)
    isDead = tmpIsDead

    -- クリアしていなければ早期リターン
    if not isClear then
        return
    end    

    if not isPlay then
        return
    end

    -- シーン切り替えの演出を実行する
    if runType == 0 then
        FadeIn()
    elseif runType == 1 then
        Wait()
    elseif runType == 2 then
        FadeOut()
    end

end

-- 入りの時間
function FadeIn()

    isRun = true

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / fadeInTime,1.0)

    mat.color.w = QFE.Math.Leap(1.0,0.0,t)

    if timer >= fadeInTime then
        runType = 1
        timer = 0.0
        mat.color.w = 1.0
        -- リセットを有効にする
        isPreReset = true
        isReset = false
    end

end

-- 待ち時間
function Wait()

    if isPreReset then
        isPreReset = false
        fremeCount = 0
        hasSentReset = false
    else
         fremeCount = fremeCount + 1

        if fremeCount == 2 and not hasSentReset then
            isReset = true
            hasSentReset = true
            DebugLog("isReset : true")
        elseif fremeCount >= 3 then
            isReset = false
        end
    end

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    if timer >= waitTime then
        runType = 2
        
        timer = 0.0
    end

end

-- 終わりの時間
function FadeOut()
    isReset = false

    local deltatime = GetDeltaTime()
    timer = timer + deltatime

    -- 0から1の範囲に収める
    local t = math.min(timer / fadeOutTime,1.0)

    mat.color.w = QFE.Math.Leap(0.0,1.0,t)

    if timer >= fadeOutTime then
        timer = 0.0
        mat.color.w = 0.0

        runType = 0
        isRun = false
        intervalTimer = 0.0
    end

end
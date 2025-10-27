damageInterval = 0.0
maxDamageInterval = 0.5
hp = 5
maxHp = 5

local time = 0.0

stoneName = "hakaName"

-- 生存フラグ
isAlive = true
-- 敵のタイプ番号
enemyType = 0
-- 普通の敵の名前
normalEnemyName = "NormalGhostEnemy.json"

-- カメラの名前
targetName = "DamageBorder"
local targetId = 0
local targetTransform = Transform.new()

-- 音
local spawn = QFE.Audio.LoadSound("tomGen.mp3")

function Init()
    isAlive = true
    hp = maxHp
    damageInterval = 0.0
    time = 0.0

     -- 追跡するidを取得
    targetId = GetEntity(targetName)
end

function Update()

  if isAlive then 

      time = time + 1.0

      if damageInterval > 0.0 then
          damageInterval = damageInterval - 0.016
          transform.scale.y = math.sin(time) * math.sin(time)
      else
          transform.scale.y = 1.0
      end
    
    -- カメラの追跡する位置を設定
    targetTransform = GetTransform(targetId)
    local targetX = targetTransform.translate.x

    if enemyType == 0 then
        -- 画面左端を出たら生存フラグをfalse
        if transform.translate.x <= targetX then
            DebugLog("EnemyIsAliveFalse")
            isAlive = false
            -- 通常の敵だった場合、通常の幽霊を生成
            if enemyType == 0 then
                local temp = Transform.new()
                temp.translate.x = transform.translate.x + 1.0
                temp.translate.y = transform.translate.y
                temp.translate.z = transform.translate.z
                CreateEntity(normalEnemyName,temp)
                CreateEntity("EnemyChangeEmitter.json",transform)
            end
        end
    else
        -- 画面左端を出たら生存フラグをfalse
        if transform.translate.x <= targetX - 4.0 then
            DebugLog("EnemyIsAliveFalse")
            isAlive = false
            -- 通常の敵だった場合、通常の幽霊を生成
            if enemyType == 0 then
                local temp = Transform.new()
                temp.translate.x = transform.translate.x + 1.0
                temp.translate.y = transform.translate.y
                temp.translate.z = transform.translate.z
                CreateEntity(normalEnemyName,temp)
            end
        end
    end

    -- 前に行き過ぎていたら削除する
    if transform.translate.x >= targetX + 30.0 then
        isAlive = false
    end
    
  end
end

function OnCollisionEnter(id,obj)
    if obj.tag == "bullet" then
        hp = hp - 1
        
        if damageInterval <= 0.0 then
            --CreateEntity("ExplotionParticleEmitter.json",transform)
            CreateEntity("EnemyHitEmitter.json",transform)
            damageInterval = maxDamageInterval
        end
        
        if hp <= 0 then
            if isAlive then
                isAlive = false
                if enemyType == 0 then
                    CreateEntity("BreakEnemyEmitter.json",transform)
                    -- 通常の敵の場合、墓石を生成
                    local temp = Transform.new()
                    temp.translate.x = transform.translate.x
                    temp.translate.y = transform.translate.y + 2.0
                    temp.translate.z = transform.translate.z
                    CreateEntity(stoneName,temp)
                    QFE.Audio.PlaySound(spawn,false,0.2)
                elseif enemyType == 2 then 
                    -- 双子のゴーストの場合
                    local temp = Transform.new()
                    temp.translate.x = transform.translate.x
                    temp.translate.y = transform.translate.y
                    temp.translate.z = transform.translate.z - 0.5
                    CreateEntity(normalEnemyName,temp)
                    local temp2 = Transform.new()
                    temp2.translate.x = transform.translate.x
                    temp2.translate.y = transform.translate.y
                    temp2.translate.z = transform.translate.z + 0.5
                    CreateEntity(normalEnemyName,temp2)
                end
            end
        end
    end
end
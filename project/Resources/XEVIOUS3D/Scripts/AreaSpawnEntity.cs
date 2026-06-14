using System;
using QuickForgeEngine;

public class AreaSpawnEntity : QuickForgeComponent
{
    float spawnDistance = 25.0f;
    string borderEntityName = "GameAreaManager";
    uint borderEntityID;
    public override void Initialize(float deltaTime)
    {
        borderEntityID = Entity.GetEntityFromName(borderEntityName);
    }
    public override void Update(float deltaTime)
    {
        Vector3 borderPos = GetPositionFromEntity(borderEntityID);

        // ボーダーとの距離がspawnDistance以下なら動く
        if (transform_.translate.z - borderPos.z < spawnDistance)
        {
            Logic.CreateEntity(GetTag(),transform_.translate);
            Destroy();
            return;
        }
    }
}
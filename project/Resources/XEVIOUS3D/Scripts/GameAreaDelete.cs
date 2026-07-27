using System;
using QuickForgeEngine;

public class GameAreaDelete : QuickForgeComponent
{
    float speed = 1.0f;
    float moveXRange = 15.0f;
    uint borderEntityID;
    string borderEntityName = "GameAreaManager";
    public override void Initialize(float deltaTime)
    {
        borderEntityID = Entity.GetEntityFromName(borderEntityName);
    }
    public override void Update(float deltaTime)
    {
        Vector3 borderPos = GetPositionFromEntity(borderEntityID);

        // Xの距離がmoveXRange以上なら消える
        if (Math.Abs(transform_.translate.x - borderPos.x) > moveXRange)
        {
            Destroy();
            return;
        }

        // ボーダーを超えたら消える
        if (transform_.translate.z < (borderPos.z-3.0f))
        {
            Destroy();
            return;
        }
    }
}
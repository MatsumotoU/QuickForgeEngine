using System;
using QuickForgeEngine;

public class EnemyInOutMove : QuickForgeComponent
{
    float speed = 1.0f;

    float moveInDistance = 25.0f;

    float moveOutDistance = 8.0f;

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

        // ボーダーとの距離がmoveOutDistance以上なら動く
        if (transform_.translate.z - borderPos.z < moveOutDistance)
        {
            float outDirX = borderPos.x - transform_.translate.x;
            transform_.translate.x -= speed * deltaTime * outDirX;
        }

        // ボーダーとの距離がmoveInDistance以下なら動く
        if (transform_.translate.z - borderPos.z < moveInDistance)
        {
            transform_.translate.z -= speed * deltaTime;
        }
    }
}
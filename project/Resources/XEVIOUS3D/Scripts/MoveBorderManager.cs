using System;
using QuickForgeEngine;

public class MoveBorderManager : QuickForgeComponent
{
    string playerEntityName = "Player";
    uint playerEntityID;

    float borderSpeed = 1.0f;
    float borderZMax = 10.0f;

    float goalZ = 60.0f;

    public override void Initialize(float deltaTime)
    {
        playerEntityID = Entity.GetEntityFromName(playerEntityName);
    }
    public override void Update(float deltaTime)
    {
        if(transform_.translate.z > goalZ)
        {
            return;
}

        float borderZ = transform_.translate.z;
        Vector3 playerPos = GetPositionFromEntity(playerEntityID);
        float speedRate =1.0f;
        
        if (playerPos.z < borderZ)
        {
            Vector3 pos = transform_.translate;
            Vector3 targetPos = playerPos;
            targetPos.z = borderZ;
            SetPositionFromEntity(playerEntityID, targetPos);
            speedRate = 0.5f;
        }

        if (playerPos.z > borderZ + borderZMax)
        {
            Vector3 pos = transform_.translate;
            pos.z = playerPos.z - borderZMax;
            transform_.translate = pos;
            speedRate = 1.5f;
        }

        Vector3 borderPos = transform_.translate;
        borderPos.z += borderSpeed * deltaTime * speedRate;
        transform_.translate = borderPos;
    }
}
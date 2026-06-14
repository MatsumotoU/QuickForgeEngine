using System;
using QuickForgeEngine;

public class CameraManager : QuickForgeComponent
{
    string borderEntityName = "GameAreaManager";
    uint borderEntityID;

    public override void Initialize(float deltaTime)
    {
        borderEntityID = Entity.GetEntityFromName(borderEntityName);
    }
    public override void Update(float deltaTime)
    {
        Vector3 borderPos = GetPositionFromEntity(borderEntityID);
        transform_.translate.x = borderPos.x;
        transform_.translate.z = borderPos.z - 15.0f;
        transform_.translate.y = 16.0f;
    }
}
using System;
using QuickForgeEngine;

public class TargetLook : QuickForgeComponent
{
    uint targetEntityID;
    string targetEntityName = "Player";
    public override void Initialize(float deltaTime)
    {
        targetEntityID =Entity.GetEntityFromName(targetEntityName);
    }
    public override void Update(float deltaTime)
{
    Vector3 targetPos = GetPositionFromEntity(targetEntityID);
    Vector3 dir =  targetPos-transform_.translate;
    
    float targetAngle = (float)Math.Atan2(dir.x, dir.z);
    
    Vector3 rot = transform_.rotate;
    float currentAngle = rot.y;
    
    float diff = targetAngle - currentAngle;
    while (diff > Math.PI) diff -= (float)(2.0 * Math.PI);
    while (diff < -Math.PI) diff += (float)(2.0 * Math.PI);
    
    rot.y = currentAngle + diff * Math.Min(1.0f, deltaTime * 5.0f);
    transform_.rotate = rot;
}
}
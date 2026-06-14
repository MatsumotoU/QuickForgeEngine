using System;
using QuickForgeEngine;

public class RotateY : QuickForgeComponent
{
    float speed = 5.0f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        // 回転処理
        Vector3 rot = transform_.rotate;
        rot.y += deltaTime * speed;
        transform_.rotate = rot;
    }
}
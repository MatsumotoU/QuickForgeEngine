using System;
using QuickForgeEngine;

public class RotateX : QuickForgeComponent
{
    float speed = 3.0f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        // 回転処理
        Vector3 rot = transform_.rotate;
        rot.x -= deltaTime * speed;
        transform_.rotate = rot;
    }
}
using System;
using QuickForgeEngine;

public class MoveZm : QuickForgeComponent
{
    float speed = 1.5f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        transform_.translate.z -= speed * deltaTime;
    }
}
using System;
using QuickForgeEngine;

public class RotateY : QuickForgeComponent
{

    public override void Initialize(float deltaTime)
    {
        
    }
    public override void Update(float deltaTime)
    {
        transform_.rotate.y += deltaTime;
    }
}
using System;
using QuickForgeEngine;

public class TimeScale : QuickForgeComponent
{
    float speed = 3.0f;
    float amplitude = 0.1f;

    float time = 0.0f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        time += deltaTime;
        
        transform_.scale.x = 1.0f + (float)Math.Sin(time * speed) * amplitude;
        transform_.scale.y = 1.0f + (float)Math.Sin(time * speed) * amplitude;
        transform_.scale.z = 1.0f + (float)Math.Sin(time * speed) * amplitude;
    }
}
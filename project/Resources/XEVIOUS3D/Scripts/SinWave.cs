using System;
using QuickForgeEngine;

public class SinWave : QuickForgeComponent
{
    float speed = 1.0f;
    float amplitude = 0.1f;

    float time = 0.0f;

    float baseY = -0.8f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        time += deltaTime;
        Vector3 pos = transform_.translate;
        pos.y = baseY + (float)Math.Sin(time * speed) * amplitude;
        transform_.translate = pos;
    }
}
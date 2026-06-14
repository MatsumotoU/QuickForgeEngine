using System;
using QuickForgeEngine;

public class GrayScaleTest : QuickForgeComponent
{
    float time = 0.0f;
    float maxTime = 10.0f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        time += deltaTime;
        float t = time / maxTime;
        PostEffect.SetGrayscale(t);
        if (time > maxTime)
        {
            PostEffect.SetGrayscale(1.0f);
            Destroy();
        }
    }
}
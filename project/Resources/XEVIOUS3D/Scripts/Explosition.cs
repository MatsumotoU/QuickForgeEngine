using System;
using QuickForgeEngine;

public class Explosition : QuickForgeComponent
{
    float aliveTime = 2.0f;
    float radius = 2.0f;
    public override void Initialize(float deltaTime)
    {
        Audio.PlaySound(Audio.LoadSound("explosion.wav"),false,0.5f);
    }
    public override void Update(float deltaTime)
    {
        aliveTime -= deltaTime;
        if (aliveTime <= 0.0f)
        {
            Destroy();
            return;
        }

        Vector3 scale = transform_.scale;
        if(aliveTime > 0.5f){
            scale.x = Logic.ExponentialDecay(scale.x, radius, 0.1f);
            scale.y = Logic.ExponentialDecay(scale.y, radius, 0.1f);
            scale.z = Logic.ExponentialDecay(scale.z, radius, 0.1f);
        }else{
            scale.x = Logic.ExponentialDecay(scale.x, 0.0f, 0.3f);
            scale.y = Logic.ExponentialDecay(scale.y, 0.0f, 0.3f);
            scale.z = Logic.ExponentialDecay(scale.z, 0.0f, 0.3f);
        }
        transform_.scale = scale;

        Vector3 rot = transform_.rotate;
        rot.x += 4.5f * deltaTime;
        rot.y += 5.5f * deltaTime;
        transform_.rotate = rot;
    }
}
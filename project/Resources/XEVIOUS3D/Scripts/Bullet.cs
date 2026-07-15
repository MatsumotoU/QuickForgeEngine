using System;
using QuickForgeEngine;

public class Bullet : QuickForgeComponent
{
    float speed = 10.0f;
    float lifeTime = 2.0f;
    float currentLifeTime = 0.0f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        currentLifeTime += deltaTime;
        if (currentLifeTime >= lifeTime)
        {
            Destroy();
            return;
        }

        transform_.translate.z += speed * deltaTime;

        transform_.rotate.z += speed * deltaTime;
    }

    public override void OnCollisionEnter(uint other)
    {
        Destroy();
    }
}
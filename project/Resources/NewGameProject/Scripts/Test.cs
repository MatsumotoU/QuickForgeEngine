using System;
using QuickForgeEngine;

public class Tester : QuickForgeComponent
{
    
    float interval = 1.0f;
    float time = 1.0f;

    public override void Initialize(float deltaTime)
    {
        
    }
    public override void Update(float deltaTime)
    {
        if(deltaTime < 1.0f/65.0f) return;

        if(deltaTime > 1.0f/30.0f) return;
        transform_.rotate.y += deltaTime;

        if(deltaTime > 1.0f/30.0f) return;

        time -= deltaTime;
        
        if (time <= 0)
        {
            Logic.CreateEntity("TestObj.json", transform_.translate, transform_.rotate, transform_.scale);
            time = interval;
        }   
    }
}
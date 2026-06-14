using System;
using QuickForgeEngine;

public class MoveToPointY : QuickForgeComponent
{
    float speed = 10.0f;
    float pointY = 2.5f;
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        Vector3 pos = transform_.translate;
        if (pos.y < pointY)
        {
            pos.y += speed * deltaTime;
            if (pos.y > pointY)
            {
                pos.y = pointY;
            }
            transform_.translate = pos;
        }
    }
}
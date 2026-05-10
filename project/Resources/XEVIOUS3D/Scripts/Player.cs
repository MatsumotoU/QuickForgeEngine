using System;
using QuickForgeEngine;

public class Player : QuickForgeComponent
{
    float speed = 3.0f;
    public override void Initialize()
    {
        
    }
    public override void Update()
    {
        Vector2 dir;
        Input.GetKeyMoveDir(out dir);
        float deltaTime = Time.GetDeltaTime();

        Vector3 pos = transform.Translate;
        pos.x += dir.x * deltaTime * speed;
        pos.z += dir.y * deltaTime * speed;
        transform.Translate = pos;
    }
}
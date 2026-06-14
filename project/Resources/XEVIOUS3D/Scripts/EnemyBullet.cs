using System;
using System.Numerics;
using QuickForgeEngine;

public class EnemyBullet : QuickForgeComponent
{
    float speed = 5.0f;
    float lifeTime = 5.0f;
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

        // Calculate direction vector based on Y-axis rotation
        // Y rotation 0 → -Z direction, rotates around Y-axis
        float rotY = transform_.rotate.y;
        float dirX = (float)Math.Sin(rotY);
        float dirZ = (float)Math.Cos(rotY);

        // Update position based on direction and speed
        Vector3 pos = transform_.translate;
        pos.x += dirX * speed * deltaTime;
        pos.z += dirZ * speed * deltaTime;
        transform_.translate = pos;

        // Rotate Z-axis for visual effect
        Vector3 rot = transform_.rotate;
        rot.z -= speed * deltaTime;
        transform_.rotate = rot;
    }
}
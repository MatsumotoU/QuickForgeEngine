using System;
using QuickForgeEngine;

public class GroundAutoShot : QuickForgeComponent
{
    float cooldown = 3.0f;
    float currentCooldown = 0.0f;

    string bulletPrefabName = "GroundEnemyBullet.json";
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        if (currentCooldown > 0)
        {
            currentCooldown -= deltaTime;
        }

        if (currentCooldown <= 0)
        {
            Logic.CreateEntity(bulletPrefabName, transform_.translate, transform_.rotate);
            currentCooldown = cooldown;
        }
    }
}
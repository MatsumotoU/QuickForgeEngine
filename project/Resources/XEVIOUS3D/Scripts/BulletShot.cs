using System;
using QuickForgeEngine;

public class BulletShot : QuickForgeComponent
{
    float cooldown = 0.3f;
    float currentCooldown = 0.0f;

    string bulletPrefabName = "Bullet.json";
    string shotPositionEntityName = "Player";

    uint shotPositionEntityID;

    uint shotSoundID;
    public override void Initialize(float deltaTime)
    {
        shotPositionEntityID = Entity.GetEntityFromName(shotPositionEntityName);
        shotSoundID = Audio.LoadSound("shot_bullet.wav");
    }
    public override void Update(float deltaTime)
    {
        if (currentCooldown > 0)
        {
            currentCooldown -= deltaTime;
        }

        if (Input.GetKeyPress("Shot") && currentCooldown <= 0)
        {
            Vector3 shotPos = GetPositionFromEntity(shotPositionEntityID);
            Logic.CreateEntity(bulletPrefabName, shotPos);
            Audio.PlaySound(shotSoundID,false,0.5f);
            currentCooldown = cooldown;
        }
    }
}
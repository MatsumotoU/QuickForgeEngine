using System;
using QuickForgeEngine;

public class BomShot : QuickForgeComponent
{
    float cooldown = 3.0f;
    float currentCooldown = 0.0f;

    string bomPrefabName = "Bom.json";
    string shotPositionEntityName = "Player";

    uint shotPositionEntityID;

    uint shotSoundID;
    public override void Initialize(float deltaTime)
    {
        shotPositionEntityID = Entity.GetEntityFromName(shotPositionEntityName);
        shotSoundID = Audio.LoadSound("shot_bom.wav");
    }
    public override void Update(float deltaTime)
    {
        if (currentCooldown > 0)
        {
            currentCooldown -= deltaTime;
        }

        if (Input.GetKeyPress("Bom") && currentCooldown <= 0)
        {
            Vector3 shotPos = GetPositionFromEntity(shotPositionEntityID);
            Logic.CreateEntity(bomPrefabName, shotPos);
            Audio.PlaySound(shotSoundID,false,0.5f);
            currentCooldown = cooldown;
        }
    }
}
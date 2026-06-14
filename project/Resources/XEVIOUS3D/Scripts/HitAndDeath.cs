using System;
using QuickForgeEngine;

public class HitAndDeath : QuickForgeComponent
{
    string explositionPrefabName = "Explosition.json";

    float invincibilityTime = 0.5f;
    float currentInvincibilityTime = 0.0f;
    bool isDead = false;

    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        if (!isDead) return;
        transform_.rotate.y += 5.0f * deltaTime;
        float t = currentInvincibilityTime / invincibilityTime;
        transform_.scale.x = t;
        transform_.scale.y = t;
        transform_.scale.z = t;

        if (currentInvincibilityTime > 0.0f)
        {
            currentInvincibilityTime -= deltaTime;
            return;
        }
        Destroy();
    }

    public override void OnCollisionEnter(uint otherEntityID)
    {
        if(isDead) return;
        isDead = true;
        Audio.PlaySound(Audio.LoadSound("explosion.wav"),false,0.5f);
        currentInvincibilityTime = invincibilityTime;
    }

    public override void OnCollisionStay(uint otherEntityID)
    {
        
    }
}
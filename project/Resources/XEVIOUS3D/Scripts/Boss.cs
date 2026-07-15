using System;
using QuickForgeEngine;

public class Boss : QuickForgeComponent
{
    bool isDamage = false;
    bool isDead = false;
    int hp = 30;

    float invincibilityTime = 3.0f;
    float currentInvincibilityTime = 0.0f;

    string explositionPrefabName = "Explosition.json";

    float time = 0.0f;

    Vector3 startPos = new Vector3();
    public override void Initialize(float deltaTime)
    {
        startPos.x = 0.0f;
        startPos.y = 0.0f;
        startPos.z = 75.0f;
    }
    public override void Update(float deltaTime)
    {
        time += deltaTime;

        isDamage = false;
        if (isDead)
        {
            transform_.rotate.x += 0.2f * deltaTime;
            transform_.rotate.y += 0.2f * deltaTime;
            transform_.rotate.z += 0.2f * deltaTime;

            transform_.translate.y -= 0.9f * deltaTime;

            if (currentInvincibilityTime > 0.0f)
            {
                currentInvincibilityTime -= deltaTime;
                return;
            }

            SceneManager.LoadScene("ResultScene.json");
            Destroy();
        }

        // 左右に8の字に動く
        Vector3 pos = transform_.translate;
        pos.x = (float)Math.Sin(time) * 6.0f;
        pos.z = (float)Math.Cos(time) * 3.0f;
        transform_.translate = pos+startPos;

    }

    public override void OnCollisionEnter(uint otherEntityID)
    {
        if (isDead) return;
        if (isDamage) return;
        hp -= 1;
        isDamage = true;
        Audio.PlaySound(Audio.LoadSound("hit.wav"), false, 0.5f);
        transform_.translate.z += 1.0f;

        if (hp <= 0)
        {
            isDead = true;
            currentInvincibilityTime = invincibilityTime;

            Logic.CreateEntity(explositionPrefabName, transform_.translate);
        }
    }
}
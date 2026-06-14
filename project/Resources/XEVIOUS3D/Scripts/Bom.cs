using System;
using QuickForgeEngine;

public class Bom : QuickForgeComponent
{
    float time = 0.0f;
    float explosionHeight = 0.5f;
    string explositionPrefabName = "Explosition.json";
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        if(transform_.translate.y < explosionHeight){
            Logic.CreateEntity(explositionPrefabName, transform_.translate);
            Destroy();
            return;
        }

        Vector3 rot = transform_.rotate;
        rot.x += 0.3f * deltaTime;
        transform_.rotate = rot;
    }
}
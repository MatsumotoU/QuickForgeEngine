using System;
using QuickForgeEngine;
public class BgmPlayer : QuickForgeComponent
{
    bool isPlaying = false;
    public override void Initialize(float deltaTime)
    {
        
    }
    public override void Update(float deltaTime)
    {
        if (!isPlaying)
        {
            Audio.PlaySound(Audio.LoadSound(GetTag()),true,0.3f);
            isPlaying = true;
        }
    }
}
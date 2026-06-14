using System;
using QuickForgeEngine;

public class TagSceneJump : QuickForgeComponent
{
    public override void Initialize(float deltaTime)
    {
    }
    public override void Update(float deltaTime)
    {
        if(Input.GetKeyTrigger("Decide"))
        {
            SceneManager.LoadScene(GetTag());
        }
    }
}
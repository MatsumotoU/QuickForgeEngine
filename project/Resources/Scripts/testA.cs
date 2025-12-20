using System;
using QuickForgeEngine;

public class testA : QuickForgeComponent
{
    public float time = 0.0f;
    public override void Initialize()
    {
        Debug.Log(this.EntityID.ToString());
    }
    public override void Update(){
        time += Time.GetDeltaTime();
        if(Time.GetDeltaTime() < 0.03f && time > 1.0f)
        {
            Entity.Create("CsharpTestObj.json");
            time = 0.0f;
        }

        this.transform.Rotate += new Vector3 { x = 0, y = 0.1f, z = 0 };
    }
}

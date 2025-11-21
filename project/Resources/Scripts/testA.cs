using System;
using QuickForgeEngine;

public class testA : QuickForgeComponent
{
    public override void Initialize()
    {
        Debug.Log(this.EntityID.ToString());
    }
    public override void Update(){
        this.transform.Rotate += new Vector3 { x = 0, y = 0.1f, z = 0 };
    }
}

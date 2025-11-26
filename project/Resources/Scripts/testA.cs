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
        Vector3 test = new Vector3 { x = 1.0f, y = 1.0f, z = 1.0f };
        Debug.Log("Length: " + test.Normalize().ToString());

        this.transform.Rotate += new Vector3 { x = 0, y = 0.1f, z = 0 };
    }
}

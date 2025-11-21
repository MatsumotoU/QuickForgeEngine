using System;
using QuickForgeEngine;

public class testA : QuickForgeComponent
{
    public override void Initialize()
    {
        Debug.Log(this.EntityID.ToString());
    }
    public override void Update(){
        if(Input.GetKeyTrigger("Jump"))
        {
            Debug.Log("Jump Pressed");
        }
        if(Input.GetKeyPress("Jump"))
        {
            Debug.Log("Jump Holding");
        }
        if (Input.GetKeyRelease("Jump"))
        {
            Debug.Log("Jump Released");
        }

        this.transform.Rotate += new Vector3 { x = 0, y = 0.1f, z = 0 };
    }
}

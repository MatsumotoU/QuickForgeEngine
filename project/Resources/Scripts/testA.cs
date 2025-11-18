using System;
using QuickForgeEngine;

public class testA : QuickForgeComponent
{
    public override void Initialize()
    {
        Debug.Log(this.EntityID.ToString());
    }
    public override void Update(){}
}

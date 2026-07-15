using System;
using QuickForgeEngine;

public class PlayerReticle : QuickForgeComponent
{
    string playerName = "Player";
    uint playerID;
    public override void Initialize(float deltaTime)
    {
        playerID = Entity.GetEntityFromName(playerName);
        Debug.Log("playerID: " + playerID.ToString());
    }
    public override void Update(float deltaTime)
    {
        Vector3 pos = this.transform_.translate;
        Vector3 playerPos = GetPositionFromEntity(playerID);
        pos.x = playerPos.x;
        pos.z = playerPos.z + 9.5f;
        this.transform_.translate = pos;
    }
}
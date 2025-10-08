function Init()

end

function Update()

end

function OnCollisionStay(idA,idB,objA,objB)
    DebugLog("Hit")
    if  objA.tag == "Player" then
        DebugLog("objA: tag.Player")
    end
    DebugLog(objA.name)

    DebugLog("EndHit")
end
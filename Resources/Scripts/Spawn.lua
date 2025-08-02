function Init()
	DebugLog("Init")

	DebugLog("type(asset): " .. tostring(type(asset)))
	DebugLog("tostring(asset): " .. tostring(asset))
	asset:CallAssetFile("anchor[1].json")
	DebugLog("Go")
end

function Update()
	DebugLog("Update")
end

function Collision()
	DebugLog("IsHit")
end


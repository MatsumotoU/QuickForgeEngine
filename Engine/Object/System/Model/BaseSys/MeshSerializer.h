#pragma once
#include "Utility/SimpleJson.h"
#include "Object/Component/Data/MeshComponent.h"
class EngineCore;

class MeshSerializer {
public:
	// シリアライズ
	static nlohmann::json Serialize(const MeshComponent& meshComponent);
	// デシリアライズ
	static void Deserialize(EngineCore* engineCore, MeshComponent& meshComponent, const nlohmann::json& j);
};
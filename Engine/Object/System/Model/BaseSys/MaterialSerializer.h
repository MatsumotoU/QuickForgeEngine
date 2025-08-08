#pragma once
#include "Utility/SimpleJson.h"
#include "Object/Component/Data/MaterialComponent.h"

class EngineCore;

class MaterialSerializer {
public:
	// シリアライズ
	static nlohmann::json Serialize(const MaterialComponent& materialComponent);
	// デシリアライズ
	static void Deserialize(EngineCore* engineCore,MaterialComponent& materialComponent, const nlohmann::json& j);
};

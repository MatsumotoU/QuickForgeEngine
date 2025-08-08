#pragma once
#include "Utility/SimpleJson.h"
#include "Object/Component/Data/ParentComponent.h"

class ModelSerializer {
public:
	// シリアライズ
	static nlohmann::json Serialize(const ParentComponent& parent);
	// デシリアライズ
	static void Deserialize( ParentComponent& parent, const nlohmann::json& j);
};
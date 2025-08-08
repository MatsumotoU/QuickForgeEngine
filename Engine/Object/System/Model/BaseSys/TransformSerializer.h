#pragma once
#include "Utility/SimpleJson.h"
#include "Object/Component/Data/TransformComponent.h"
class EngineCore;

class TransformSerializer {
public:
	// シリアライズ
	static nlohmann::json Serialize(const TransformComponent& transformComponent);
	// デシリアライズ
	static void Deserialize(EngineCore* engineCore, TransformComponent& transformComponent, const nlohmann::json& j);
};
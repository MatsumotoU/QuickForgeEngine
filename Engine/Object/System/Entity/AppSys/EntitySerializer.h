#pragma once
#include "Utility/SimpleJson.h"
class EntityManager;
class EngineCore;

class EntitySerializer {
public:
	// シリアライズ
	static nlohmann::json Serialize(const EntityManager& entityManager);
	// デシリアライズ
	static void Deserialize(EngineCore* engineCore, EntityManager& entityManager, const nlohmann::json& j);
};
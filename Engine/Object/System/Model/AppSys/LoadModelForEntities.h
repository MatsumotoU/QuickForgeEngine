#pragma once
#include <string>
#include <cstdint>

class EngineCore;
class EntityManager;

class LoadModelForEntities {
public:
	LoadModelForEntities() = default;
	~LoadModelForEntities() = default;
	// エンティティマネージャーからモデルをロード
	static void Load(EngineCore* engineCore,EntityManager& entityManager, uint32_t entityNum,const std::string& modelFileName);
};
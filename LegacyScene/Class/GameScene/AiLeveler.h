#pragma once
#include "Base/EngineCore.h"

class AiLeveler {
public:
	AiLeveler();
	~AiLeveler() = default;
	
	void LoadAIConfig(const std::string& filePath);
	uint32_t GetAILevel() const { return aiLevel_; }
	float GetLifeWeight() const { return lifeWeight_; }
	float GetAttackWeight() const { return attackWeight_; }
	float GetUniqueWeight() const { return uniqueWeight_; }

private:

	uint32_t aiLevel_;
	float lifeWeight_;
	float attackWeight_;
	float uniqueWeight_;
};
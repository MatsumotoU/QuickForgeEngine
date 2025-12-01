#pragma once
#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

#include "Utility/memory/SparseSets.h"

class ParticleAnimationManager {
public:
	uint32_t LoadAnimationData(const std::string& fileName);
	void SaveAnimationData(const std::string& fileName, const nlohmann::json& animationData);

	void Initalize();
	void Finalize();
private:
	SparseSet<nlohmann::json> animationDataSet_;
};
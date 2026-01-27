#pragma once
#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

#include "engine/include/utility/memory/SparseSets.h"

namespace QFE {

	class ParticleAnimationManager {
	public:
		uint32_t LoadAnimationData(const std::string& fileName);
		void SaveAnimationData(const std::string& fileName, const nlohmann::json& animationData);

		void Initialize();
		void Finalize();
	private:
		SparseSet<nlohmann::json> animationDataSet_;
	};

}

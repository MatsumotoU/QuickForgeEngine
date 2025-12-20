#include "engine/include/assets/Particle/ParticleAnimationManager.h"
#include "engine/include/assets/AssetManager.h" 
#include <fstream>

uint32_t ParticleAnimationManager::LoadAnimationData(const std::string& fileName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// 繧ｷ繝ｼ繝ｳ繝輔ぃ繧､繝ｫ縺ｮ繝代せ繧堤ｵ・∩遶九※
	std::string filePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("ParticleAnim");
	std::ifstream ifs(filePath + fileName);
	if (!ifs.is_open()) {
		assert(false && "FaildOpenFile");
	}
	nlohmann::json animationJson;
	ifs >> animationJson;
	ifs.close();
    return 0;
}

void ParticleAnimationManager::SaveAnimationData(const std::string& fileName, const nlohmann::json& animationData) {
	fileName; animationData;
}

void ParticleAnimationManager::Initalize() {
	animationDataSet_.clear();
}

void ParticleAnimationManager::Finalize() {
	animationDataSet_.clear();
}

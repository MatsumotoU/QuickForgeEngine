#include "ParticleAnimationManager.h"
#include "Assets/AssetManager.h" 
#include <fstream>

uint32_t ParticleAnimationManager::LoadAnimationData(const std::string& fileName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// シーンファイルのパスを組み立て
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
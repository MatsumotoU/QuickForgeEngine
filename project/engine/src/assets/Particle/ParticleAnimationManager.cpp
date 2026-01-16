/**
 * @file ParticleAnimationManager.cpp
 * @brief パーティクルアニメーションのデータを管理するクラスの実装
 */

#include "engine/include/assets/Particle/ParticleAnimationManager.h"
#include "engine/include/assets/AssetManager.h" 
#include <fstream>

/**
 * @brief アニメーションデータの読み込み
 * TODO: 読み込んだデータを保持する実装が足りていない可能性が高い。
 */
uint32_t ParticleAnimationManager::LoadAnimationData(const std::string& fileName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// ファイルのパスを組み立て
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

/** @brief アニメーションデータの保存 */
void ParticleAnimationManager::SaveAnimationData(const std::string& fileName, const nlohmann::json& animationData) {
	fileName; animationData;
}

void ParticleAnimationManager::Initialize() {
	animationDataSet_.clear();
}

void ParticleAnimationManager::Finalize() {
	animationDataSet_.clear();
}

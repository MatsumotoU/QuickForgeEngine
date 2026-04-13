/**
 * @file ResourceDirectoryManager.cpp
 * @brief リソースタイプと対応するディレクトリを管理するクラスの実装
 */

#include "engine/include/assets/ResourceDirectoryManager.h"
#include <cassert>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

/// @brief すべてのリソースタイプとそのディレクトリを初期化するコンストラクタ
ResourceDirectoryManager::ResourceDirectoryManager() {
	rootDirectory_ = "Resources/";

	resourceDirectories_["Model"] = "Models/";
	resourceDirectories_["Image"] = "Images/";
	resourceDirectories_["Font"] = "Fonts/";
	resourceDirectories_["Scenes"] = "Scenes/";
	resourceDirectories_["Sounds"] = "Sounds/";
	resourceDirectories_["Scripts"] = "Scripts/";
	resourceDirectories_["Entities"] = "Entities/";
	resourceDirectories_["Config"] = "Config/";
	resourceDirectories_["2DMap"] = "2DMap/";
	resourceDirectories_["ParticleAnim"] = "ParticleAnimation/";
	resourceDirectories_["Animation"] = "Animation/";
#ifdef QFE_OPTIMIZE_OFF
	resourceDirectories_["Editor"] = "Editor/Resource/Images/";
#endif // QFE_OPTIMIZE_OFF

#ifdef QFE_OPTIMIZE_OFF
	for (const auto& [key, value] : resourceDirectories_) {
		DebugLog(std::format("Key: {},Directory: {}", key, value));
	}
#endif // QFE_OPTIMIZE_OFF

	ProjectName_ = "NewGameProject";
}

void QFE::ResourceDirectoryManager::CreateProjectDirectory(const std::string& projectName) const {
	std::string projectPath = rootDirectory_ + projectName + "/";
	std::filesystem::create_directories(projectPath);
	for (const auto& [key, value] : resourceDirectories_) {
		std::filesystem::create_directories(projectPath + value);
	}
}

void QFE::ResourceDirectoryManager::SetProjectDirectory(const std::string& projectName) {
	std::string projectPath = rootDirectory_ + projectName + "/";
	if (!std::filesystem::exists(projectPath)) {
		CreateProjectDirectory(projectName);
	}
	ProjectName_ = projectName;
}

std::string QFE::ResourceDirectoryManager::GetProjectDirectory() const {
	return rootDirectory_ + ProjectName_;
}

/// @brief 指定されたリソースタイプに対応するディレクトリを取得する関数
std::string ResourceDirectoryManager::GetResourceDirectory(const std::string& resourceType) const {
	assert(resourceDirectories_.find(resourceType) != resourceDirectories_.end() && "Resource type not found");
	std::string directory = rootDirectory_ + ProjectName_ + "/" + resourceDirectories_.at(resourceType);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("GetResourceDirectory: ResourceType: {}, Directory: {}", resourceType, directory));
#endif // QFE_OPTIMIZE_OFF

	return directory;
}

std::string QFE::ResourceDirectoryManager::GetEditorResourceDirectory() const {
	return "Editor/Resource/Images/";
}

bool QFE::ResourceDirectoryManager::CheckDirectoryIntegrity() const {
	std::string projectPath = rootDirectory_ + ProjectName_ + "/";
	if (!std::filesystem::exists(projectPath)) {
		return false;
	}
	for (const auto& [key, value] : resourceDirectories_) {
		if (!std::filesystem::exists(projectPath + value)) {
			return false;
		}
	}
	return true;
}

void QFE::ResourceDirectoryManager::RepairDirectoryIntegrity() const {
	std::string projectPath = rootDirectory_ + ProjectName_ + "/";
	if (!std::filesystem::exists(projectPath)) {
		std::filesystem::create_directories(projectPath);
	}
	for (const auto& [key, value] : resourceDirectories_) {
		if (!std::filesystem::exists(projectPath + value)) {
			std::filesystem::create_directories(projectPath + value);
		}
	}
}

#include "ResourceDirectoryManager.h"
#include <cassert>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

ResourceDirectoryManager::ResourceDirectoryManager() {
	resourceDirectories_["Model"] = "Resources/Models/";
	resourceDirectories_["Image"] = "Resources/Images/";
	resourceDirectories_["Font"] = "Resources/Fonts/";
	resourceDirectories_["Scenes"] = "Resources/Scenes/";
	resourceDirectories_["Sounds"] = "Resources/Sounds/";

#ifdef _DEBUG
	for (const auto& [key, value] : resourceDirectories_) {
		DebugLog(std::format("Key: {},Directory: {}", key, value));
	}
#endif // _DEBUG
}

std::string ResourceDirectoryManager::GetResourceDirectory(const std::string& resourceType) const {
	assert(resourceDirectories_.find(resourceType) != resourceDirectories_.end() && "Resource type not found");
	return resourceDirectories_.at(resourceType);
}

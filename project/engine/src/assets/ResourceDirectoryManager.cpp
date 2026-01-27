/**
 * @file ResourceDirectoryManager.cpp
 * @brief 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ遞ｮ鬘槭＃縺ｨ縺ｮ繝・ぅ繝ｬ繧ｯ繝医Μ繝代せ繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ縺ｮ螳溯｣・
 */

#include "engine/include/assets/ResourceDirectoryManager.h"
#include <cassert>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

/** @brief 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ縲よｨ呎ｺ也噪縺ｪ繝・ぅ繝ｬ繧ｯ繝医Μ繝代せ繧堤匳骭ｲ縺吶ｋ縲・*/
ResourceDirectoryManager::ResourceDirectoryManager() {
	resourceDirectories_["Model"] = "Resources/Models/";
	resourceDirectories_["Image"] = "Resources/Images/";
	resourceDirectories_["Font"] = "Resources/Fonts/";
	resourceDirectories_["Scenes"] = "Resources/Scenes/";
	resourceDirectories_["Sounds"] = "Resources/Sounds/";
	resourceDirectories_["Scripts"] = "Resources/Scripts/";
	resourceDirectories_["Entities"] = "Resources/Entities/";
	resourceDirectories_["Config"] = "Resources/Config/";
	resourceDirectories_["2DMap"] = "Resources/2DMap/";
	resourceDirectories_["Project"] = "Resources/Projects/";
	resourceDirectories_["ParticleAnim"] = "Resources/ParticleAnimation/";
	resourceDirectories_["Animation"] = "Resources/Animation/";
#ifdef QFE_OPTIMIZE_OFF
	resourceDirectories_["Editor"] = "Editor/Resource/Images/";
#endif // QFE_OPTIMIZE_OFF

#ifdef QFE_OPTIMIZE_OFF
	for (const auto& [key, value] : resourceDirectories_) {
		DebugLog(std::format("Key: {},Directory: {}", key, value));
	}
#endif // QFE_OPTIMIZE_OFF
}

/**
 * @brief 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ遞ｮ鬘槭↓蟇ｾ蠢懊☆繧九ョ繧｣繝ｬ繧ｯ繝医Μ繝代せ繧貞叙蠕・
 * @param resourceType 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ遞ｮ鬘・萓・ "Model", "Image")
 * @return 繝・ぅ繝ｬ繧ｯ繝医Μ繝代せ
 */
std::string ResourceDirectoryManager::GetResourceDirectory(const std::string& resourceType) const {
	assert(resourceDirectories_.find(resourceType) != resourceDirectories_.end() && "Resource type not found");
	return resourceDirectories_.at(resourceType);
}

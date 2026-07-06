#pragma once
#include "scene/SceneManager.h"
#include "scene/component/AllComponent.h"

namespace QFE::FRAMEWORK {
	/// @brief シーンマネージャーを使用してエンティティを作成する関数
	uint32_t CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name);
}

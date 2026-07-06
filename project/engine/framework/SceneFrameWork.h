#pragma once
#include "scene/SceneManager.h"
#include "scene/component/AllComponent.h"

namespace QFE::FRAMEWORK {
	/// @brief シーンマネージャーを使用してエンティティを作成する関数
	uint32_t CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name);
	/// @brief シーンマネージャーを使用してエンティティを作成し、マテリアルコンポーネントを追加する関数
	uint32_t CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor);
}

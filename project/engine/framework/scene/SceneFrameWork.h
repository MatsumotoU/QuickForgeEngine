#pragma once
#include "math/MathInclude.h"
#include <unordered_map>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
	enum class DirectXResourceHandle : uint32_t;
	enum class BLASHandle : uint32_t;
}

namespace QFE::FRAMEWORK {
	/// @brief シーンマネージャーを使用してエンティティを作成する関数
	uint32_t CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name);
	/// @brief シーンマネージャーを使用してエンティティを作成し、マテリアルコンポーネントを追加する関数
	uint32_t CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor);

}

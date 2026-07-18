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
	/// @brief シーンマネージャーを生成する関数
	std::unique_ptr<QFE::SCENE::SceneManager> CreateSceneManager();
	/// @brief シーンマネージャーのフレーム終了処理を行う関数
	bool EndFrameSceneManager(QFE::SCENE::SceneManager* sceneManager);

	/// @brief シーンマネージャーを使用してエンティティを作成する関数
	uint32_t CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name);
	/// @brief シーンマネージャーを使用してエンティティを作成し、マテリアルコンポーネントを追加する関数
	uint32_t CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor);

	bool LoadSceneFromJson(QFE::SCENE::SceneManager* sceneManager, const std::string& filePath);

	/// @brief シーンマネージャーを使用してカメラのビュー行列と投影行列を更新する関数
	bool UpdateCurrentCamera(QFE::SCENE::SceneManager* sceneManager, QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief シーンマネージャーを使用してモデルのレンダーコンポーネントを更新する関数
	bool UpdateModelRenderComponents(
		QFE::SCENE::SceneManager* sceneManager, QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::MATH::Matrix4x4& viewProj);
}

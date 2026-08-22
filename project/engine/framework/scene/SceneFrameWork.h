#pragma once
#include "math/MathInclude.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include <unordered_map>
#include <vector>

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

	/// @brief メインカメラの行列を更新してview-projection行列を返す。
	QFE::MATH::Matrix4x4 UpdateMainCamera(QFE::SCENE::SceneManager& sceneManager);

	/// @brief 描画可能なModelRenderComponentを持つエンティティを描画する。
	void DrawSceneModels(
		QFE::SCENE::SceneManager& sceneManager,
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::PSOHandle psoHandle,
		QFE::GRAPHIC::ViewPortHandle viewportHandle,
		QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes);

	/// @brief 描画可能なSpriteRenderComponentを描画順に最終ターゲットへ描画する。
	void DrawSceneSprites(
		QFE::SCENE::SceneManager& sceneManager,
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::PSOHandle psoHandle,
		QFE::GRAPHIC::ViewPortHandle viewportHandle,
		QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
		QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle,
		QFE::GRAPHIC::RenderTargetHandle renderTarget);

}

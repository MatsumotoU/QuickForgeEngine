#pragma once
#include "scene/SceneManager.h"

namespace QFE::FRAMEWORK {
	// NetworkFramework と同様、データ型を共有し、操作をフレームワークに集約する。
	// 実装側の変更がアプリケーションに広がらないようにするための窓口。
	void InitializeScene(SCENE::SceneManager& scene);
	uint32_t CreateSceneCamera(SCENE::SceneManager& scene);
	uint32_t CreateSceneDirectionalLight(SCENE::SceneManager& scene);
	// ECS の所有権はシーンに残る。参照はシーンマネージャーの寿命内で使用する。
	EntityManager& GetSceneEntities(SCENE::SceneManager& scene);
	/// @brief 読み込まれた全シーンの入力制御を合わせ、アクションが許可されているか調べる。
	bool IsGameActionEnabled(SCENE::SceneManager& scene, const std::string& action);
	// 未保存時は空文字列。参照はシーンの変更をまたいで保持しない。
	const std::string& GetScenePath(const SCENE::SceneManager& scene);
	void LoadScene(SCENE::SceneManager& scene, const std::string& path);
	bool LoadScene(SCENE::SceneManager& scene, const nlohmann::json& data);
	void SaveScene(SCENE::SceneManager& scene, const std::string& path);
	// 保存前のファイル内容を退避し、保存操作を取り消せるようにする。
	nlohmann::json ReadSceneJson(SCENE::SceneManager& scene, const std::string& path);
	// シーン内容と保存先を合わせて退避し、読み込み操作を取り消せるようにする。
	SCENE::SceneSnapshot CaptureScene(SCENE::SceneManager& scene);
	bool RestoreScene(SCENE::SceneManager& scene, const SCENE::SceneSnapshot& snapshot);
	uint32_t LoadSceneEntity(SCENE::SceneManager& scene, const std::string& path);
	// シーンの原点が origin になるよう、エンティティを追加する。
	uint32_t AppendScene(SCENE::SceneManager& scene, const std::string& path, const QFE::MATH::Vector3& origin);
	// scenes フォルダーからの相対パスを、存在するシーンアセットに解決する。
	std::filesystem::path ResolveSceneSpawnAsset(const std::string& relativePath);
	// イベント等からの追加要求をまとめて処理する。
	void UpdateSceneSpawnRequests(SCENE::SceneManager& scene);
	// 現在更新中のシーンから、独立シーンの読み込みと状態変更を要求する。
	void UpdateSceneControlRequests(SCENE::SceneManager& scene);
	// プレイヤーの部屋を判定し、ゲーム側の移動処理後にカメラを移動する。
	void UpdateRoomCameras(EntityManager& entities, float deltaTime);
}


#include "math/MathInclude.h"
#include "graphics/interface/DirectXRenderFramework.h"
#include <unordered_map>
#include <vector>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::GRAPHIC {
	class IGraphicEngine;
	enum class DirectXResourceHandle : uint32_t;
	enum class BLASHandle : uint32_t;
}

namespace QFE::FRAMEWORK {
	/// @brief シーンマネージャーを使用してエンティティを作成する関数
	uint32_t CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name);
	/// @brief シーンマネージャーを使用してエンティティを作成し、マテリアルコンポーネントを追加する関数
	uint32_t CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor);

	/// @brief メインカメラの行列を更新してview-projection行列を返す。
	QFE::MATH::Matrix4x4 UpdateMainCamera(
		QFE::SCENE::SceneManager& sceneManager, QFE::MATH::Vector3* outCameraPosition = nullptr);

	/// @brief 描画可能なModelRenderComponentを持つエンティティを描画する。
	void DrawSceneModels(
		QFE::SCENE::SceneManager& sceneManager,
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::PSOHandle psoHandle,
		QFE::GRAPHIC::ViewPortHandle viewportHandle,
		QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes);

	/// @brief 描画可能なSpriteRenderComponentを描画順に最終ターゲットへ描画する。
	void DrawSceneSprites(
		QFE::SCENE::SceneManager& sceneManager,
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::PSOHandle psoHandle,
		QFE::GRAPHIC::ViewPortHandle viewportHandle,
		QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
		QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle,
		QFE::GRAPHIC::RenderTargetHandle renderTarget);

}

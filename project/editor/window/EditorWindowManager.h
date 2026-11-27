#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "IEditorWindow.h"

#include <imgui/imgui.h>
#include <unordered_map>
#include "math/MathInclude.h"
#include "framework/application/EngineConfig.h"
#include "framework/input/InputFrameWork.h"
#include "framework/network/NetworkCoreFrame.h"
#include <chrono>

namespace QFE::SCENE {
	class SceneManager;
}
namespace QFE::WINDOW {
	class IGameWindowManager;
}
namespace QFE::FRAMEWORK { class WindowsEngineResources; }
namespace QFE::ASSET { class AssetManager; class ModelLoader; }

namespace QFE::EDITOR {
	class EditorCommandList;
	class ComponentInspectorRegistry;

	enum class EditorWindowType {
		Hierarchy,
		SceneViewer,
		GameViewer,
		Inspector,
		Profiler,
		Console,
		AssetBrowser,
		Logger,
		AnimationEditor,
	};

	/// @brief エディタのウィンドウ管理クラス
	class EditorWindowManager final{
	public:
		void Initialize(
			QFE::SCENE::SceneManager* sceneManager,
			ImTextureID sceneTextureId,
			QFE::WINDOW::IGameWindowManager& windowManager,
			QFE::ASSET::AssetManager& assetManager,
			QFE::ASSET::ModelLoader& modelLoader,
			ComponentInspectorRegistry* componentInspectorRegistry,
			const QFE::FRAMEWORK::EngineConfig& config);
		void Update();
		void Draw(EditorCommandList& commandList, QFE::FRAMEWORK::WindowsEngineResources& resources);
		/// @brief Fileメニューから要求されたGameModule DLLのビルドとリロードを取得する。
		bool ConsumeGameModuleBuildAndReloadRequest();

		/// @brief 指定されたウィンドウタイプのウィンドウにフォーカスがあるかどうかを返す
		bool IsWindowFocused(EditorWindowType windowType);

		/// @brief Hierarchyからのカメラフォーカス要求を取得する
		bool ConsumeCameraFocusTarget(QFE::MATH::Vector3& position, float& radius);
		/// @brief SceneViewerで使用するカメラ行列を設定する
		void SetSceneViewerCamera(
			const QFE::MATH::Matrix4x4& viewMatrix,
			const QFE::MATH::Matrix4x4& projectionMatrix,
			bool isOrthographic = false);
		/// @brief 表示中のカメラ行列をアセットのドロップ位置計算に使用する。
		void SetAssetDropViewProjection(const QFE::MATH::Matrix4x4& viewProjection);
		bool TryGetGameViewerRoomPreview(QFE::MATH::Matrix4x4& viewProjection, QFE::MATH::Vector3& cameraPosition) const;
		/// @brief SceneViewerのギズモがマウス入力を使用しているかを返す
		bool IsSceneGizmoCapturingMouse() const;

	private:
		/// @brief 現在の保存先、またはユーザーが選択した保存先へシーン保存を追加する
		void QueueSceneSave(EditorCommandList& commandList, bool selectSavePath);
		/// @brief Hierarchyで選択されたEntityをプロジェクト内に保存する。
		void SaveSelectedEntities();

		// エディタで表示できるウィンドウたち
		std::unordered_map<EditorWindowType, std::unique_ptr<IEditorWindow>> editorWindowsMap_;
		std::set<uint32_t> selectedEntities_;
		QFE::WINDOW::IGameWindowManager* windowManager_ = nullptr;
		SCENE::SceneManager* sceneManager_;
		std::string projectName_;
		bool gameModuleBuildAndReloadRequested_ = false;
		QFE::FRAMEWORK::EngineConfig engineConfig_;
		bool showEngineConfig_ = false;
		std::string engineConfigStatus_;
		// Inspector と同じ一覧を参照し、ゲームごとのタグ選択を即座に反映する。
		std::vector<std::string> gameTags_{ "Default" };
		std::vector<QFE::FRAMEWORK::GameInputAction> gameInputActions_;
		std::string newInputAction_;
		std::string newGameTag_;
		std::string gameConfigStatus_;
		bool showGameConfig_ = false;
		// ランタイムごとに接続と未完成メッセージを破棄し、前回のログが混ざるのを防ぐ。
		std::unique_ptr<QFE::NETWORK::INetworkCore> runtimeNetwork_;
		QFE::NETWORK::ConnectHandle runtimeConnection_{};
		QFE::FRAMEWORK::NETWORK::MessageReader runtimeReader_;
		std::vector<std::string> runtimeLogs_;
		bool waitingForRuntime_ = false;
		std::chrono::steady_clock::time_point runtimeDeadline_{};
	};
}

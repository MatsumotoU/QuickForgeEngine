#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "IEditorWindow.h"

#include <imgui/imgui.h>
#include <unordered_map>
#include "math/MathInclude.h"

#define NOMINMAX
#include <Windows.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	class EditorCommandList;

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
		void Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow);
		void Update();
		void Draw(EditorCommandList& commandList);

		/// @brief 指定されたウィンドウタイプのウィンドウにフォーカスがあるかどうかを返す
		bool IsWindowFocused(EditorWindowType windowType);

		/// @brief Hierarchyからのカメラフォーカス要求を取得する
		bool ConsumeCameraFocusTarget(QFE::MATH::Vector3& position, float& radius);
		/// @brief SceneViewerで使用するカメラ行列を設定する
		void SetSceneViewerCamera(
			const QFE::MATH::Matrix4x4& viewMatrix,
			const QFE::MATH::Matrix4x4& projectionMatrix,
			bool isOrthographic = false);
		/// @brief SceneViewerのギズモがマウス入力を使用しているかを返す
		bool IsSceneGizmoCapturingMouse() const;

	private:
		/// @brief 現在の保存先、またはユーザーが選択した保存先へシーン保存を追加する
		void QueueSceneSave(EditorCommandList& commandList, bool selectSavePath);

		// エディタで表示できるウィンドウたち
		std::unordered_map<EditorWindowType, std::unique_ptr<IEditorWindow>> editorWindowsMap_;
		std::set<uint32_t> selectedEntities_;
		HWND mainWindow_;
		SCENE::SceneManager* sceneManager_;
	};
}

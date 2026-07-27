#pragma once
#include "IEditorWindow.h"
#include "math/MathInclude.h"
#include <imgui/imgui.h>
#include <ImGuizmo-1.83/ImGuizmo.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	/// @brief SceneViewerはシーンを表示します
	class SceneViewer final : public IEditorWindow {
	public:
		/// @brief SceneViewerのコンストラクタ
		SceneViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager);

		/// @brief ウィンドウの初期化処理
		void Initialize() override;
		/// @brief ウィンドウの描画処理
		void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) override;

		/// @brief ウィンドウの名前を取得する
		std::string GetWindowName() override;
		/// @brief ウィンドウがアクティブかどうかを取得する
		bool GetIsActive() override;
		/// @brief ウィンドウのアクティブ状態を設定する
		bool SetIsActive(bool isActive) override;
		/// @brief ウィンドウがフォーカスされているかどうかを取得する
		bool GetIsFocus() override;

		/// @brief SceneViewerで使用するカメラ行列を設定する
		void SetCameraMatrices(
			const QFE::MATH::Matrix4x4& viewMatrix,
			const QFE::MATH::Matrix4x4& projectionMatrix,
			bool isOrthographic = false);
		/// @brief ギズモを操作中、またはギズモ上にマウスがあるかを返す
		bool IsGizmoCapturingMouse() const;

	private:
		void DrawGizmo(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList);
		void DrawGizmoToolbar(bool imageHovered);

		SCENE::SceneManager* sceneManager_;
		ImTextureID sceneTextureId_;
		bool isActive_;
		bool isFocus_;
		bool isImageHovered_;
		bool isOrthographic_;
		bool wasUsingGizmo_;
		uint32_t editingEntityId_;
		ImGuizmo::OPERATION gizmoOperation_;
		ImGuizmo::MODE gizmoMode_;
		QFE::MATH::Matrix4x4 viewMatrix_;
		QFE::MATH::Matrix4x4 projectionMatrix_;
		QFE::MATH::EulerTransform transformBeforeEdit_;
	};
}

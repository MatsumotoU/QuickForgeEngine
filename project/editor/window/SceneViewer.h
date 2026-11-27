#pragma once
#include "IEditorWindow.h"
#include "GridPlacement.h"
#include "math/MathInclude.h"
#include "framework/gui/GuiCoreFramework.h"
#include <imgui/imgui.h>
#include <ImGuizmo-1.83/ImGuizmo.h>
#include <string>
#include <vector>

namespace QFE::SCENE {
	class SceneManager;
}
namespace QFE::ASSET { class ModelLoader; }

namespace QFE::EDITOR {
	class ComponentInspectorRegistry;
	/// @brief SceneViewerはシーンを表示します
	class SceneViewer final : public IEditorWindow {
	public:
		/// @brief SceneViewerのコンストラクタ
		SceneViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager, ASSET::ModelLoader& modelLoader, unsigned width, unsigned height,
			const ComponentInspectorRegistry* componentInspectorRegistry);

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
		/// @brief 表示中のカメラのビュー投影行列をドロップ位置計算に使用する。
		void SetAssetDropViewProjection(const MATH::Matrix4x4& viewProjection);
		void SetDebugLines(const std::vector<FRAMEWORK::DebugLine>* lines) { debugLines_ = lines; }
		// ペイント中だけブラウザの選択を受け取り、対象の種類に合わせてモードも切り替える。
		void SelectPaintAsset(const std::string& path);
		std::string ConsumePaintSelection();

	private:
		void DrawGizmo(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList);
		void DrawGizmoToolbar(bool imageHovered);
		void DrawGridControls();
		void DrawPlacementGrid(const ImVec2& imagePosition, const ImVec2& imageSize);
		void DrawPlacementTools(const ImVec2& imagePosition, const ImVec2& imageSize, EditorCommandList& commandList);
		void RefreshPlacementAssets();
		void LoadPlacementPreview();

		SCENE::SceneManager* sceneManager_;
		const ComponentInspectorRegistry* componentInspectorRegistry_;
		bool showGameOverlays_ = true;
		const std::vector<FRAMEWORK::DebugLine>* debugLines_ = nullptr;
		ImTextureID sceneTextureId_;
		unsigned width_;
		unsigned height_;
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
		QFE::MATH::Matrix4x4 dropViewProjection_ = MATH::Matrix4x4::MakeIdentity4x4();
		QFE::MATH::EulerTransform transformBeforeEdit_;
		GridPlacement grid_;
		// プレビュー用データはシーンに追加せず、保存や衝突判定の対象にしない。
		ASSET::ModelLoader* modelLoader_;
		int placementMode_ = 0;
		int placementIndex_ = 0;
		std::vector<std::string> placementAssets_;
		std::vector<MATH::Vector3> previewTriangles_;
		std::string placementStatus_;
		// 元アセットのグループを使うか、配置先を明示するかを区別する。
		bool useAssetGroups_ = true;
		std::string placementGroup_;
		float eraseRadius_ = 0.5f; // グリッド交点からエンティティ原点までのワールド距離。
		std::string changedPaintSelection_;
		MATH::Vector3 lastPaintPosition_{};
		bool hasLastPaintPosition_ = false;
		int lastPaintButton_ = -1;
	};
}

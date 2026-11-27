// 間接的に Windows.h が先に読み込まれても std::min / std::max を妨げない。
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GameEditor.h"
#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"

#include <algorithm>


using namespace QFE::EDITOR;

void QFE::EDITOR::GameEditor::Initialize(
	QFE::SCENE::SceneManager* sceneManager,
	ImTextureID sceneTextureId,
	QFE::WINDOW::IGameWindowManager& windowManager,
	QFE::ASSET::AssetManager& assetManager,
	QFE::ASSET::ModelLoader& modelLoader,
	const QFE::FRAMEWORK::EngineConfig& config) {
	windowManager_.Initialize(
		sceneManager, sceneTextureId, windowManager, assetManager, modelLoader, &componentInspectorRegistry_, config);
	commandExecutor_.Initialize();
	commandList_.ClearCommands();

	activeCameraType_ = EditorCameraType::DebugCamera;
}

void QFE::EDITOR::GameEditor::Update(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources) {

	SCENE::SceneManager& sceneManager = *systems.sceneManager;

	commandList_.ClearCommands();
	// 接続の受け入れとランタイムログの受信を毎フレーム進める。
	windowManager_.Update();

	// シーンビューがアクティブになった場合デバッグカメラに切り替える
	if (windowManager_.IsWindowFocused(EditorWindowType::SceneViewer)) {
		activeCameraType_ = EditorCameraType::DebugCamera;
	}
	// ゲームビューがアクティブになった場合メインカメラに切り替える
	if (windowManager_.IsWindowFocused(EditorWindowType::GameViewer)) {
		activeCameraType_ = EditorCameraType::MainCamera;
	}

	// Hierarchyでダブルクリックされたオブジェクトを画面中央に収める
	QFE::MATH::Vector3 focusPosition{};
	float focusRadius = 1.0f;
	if (windowManager_.ConsumeCameraFocusTarget(focusPosition, focusRadius)) {
		activeCameraType_ = EditorCameraType::DebugCamera;
		const QFE::MATH::Vector3 forward =
			QFE::MATH::TransformForwardDirection(resources.cameraTransform);
		const float focusDistance = std::max(5.0f, focusRadius * 2.5f);
		resources.cameraTransform.translate = focusPosition - forward * focusDistance;
	}

	// デバッグカメラ操作処理
	if (windowManager_.IsWindowFocused(EditorWindowType::SceneViewer) &&
		!windowManager_.IsSceneGizmoCapturingMouse() && !ImGui::GetIO().WantTextInput) {
		float moveSpeed = 0.3f;

		// 右クリックは配置に使うため、カメラ操作は中央ボタンにまとめる。
		if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
			// 移動処理
			if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
				QFE::MATH::Vector3 rotatedForward = QFE::MATH::TransformForwardDirection(resources.cameraTransform);
				resources.cameraTransform.translate += rotatedForward * moveSpeed;
			}
			if (ImGui::IsKeyDown(ImGuiKey_S) || ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
				QFE::MATH::Vector3 rotatedForward = QFE::MATH::TransformForwardDirection(resources.cameraTransform);
				resources.cameraTransform.translate -= rotatedForward * moveSpeed;
			}
			if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
				QFE::MATH::Vector3 right = QFE::MATH::TransformRightDirection(resources.cameraTransform);
				resources.cameraTransform.translate -= right * moveSpeed;
			}
			if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
				QFE::MATH::Vector3 right = QFE::MATH::TransformRightDirection(resources.cameraTransform);
				resources.cameraTransform.translate += right * moveSpeed;
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
				resources.cameraTransform.translate.y -= moveSpeed;
			}
			if (ImGui::IsKeyDown(ImGuiKey_Space)) {
				resources.cameraTransform.translate.y += moveSpeed;
			}
		}

		// 回転処理
		if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
			const ImVec2 delta = ImGui::GetIO().MouseDelta;
			// 以前のカメラ操作と同じくマウスの移動方向を使い、感度を維持する。
			QFE::MATH::Vector2 mouseMove{ delta.x, delta.y };
			mouseMove = mouseMove.Normalize();
			resources.cameraTransform.rotate.y += mouseMove.x * 0.02f;
			resources.cameraTransform.rotate.x += mouseMove.y * 0.02f;
		}
	}

	// カメラのビュー行列と投影行列を取得
	QFE::MATH::Matrix4x4& viewProj = resources.viewProj;
	QFE::MATH::Matrix4x4 viewMatrix =
		QFE::MATH::Matrix4x4::MakeAffineMatrix(resources.cameraTransform).Inverse();
	QFE::MATH::Matrix4x4 projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
		3.14159f / 4.0f, static_cast<float>(resources.windowWidth) / static_cast<float>(resources.windowHeight), 0.1f, 1000.0f);
	windowManager_.SetSceneViewerCamera(viewMatrix, projectionMatrix);

	if (activeCameraType_ == QFE::EDITOR::EditorCameraType::DebugCamera) {
		viewProj = QFE::MATH::Matrix4x4::Multiply(viewMatrix, projectionMatrix);
		resources.cameraPosition = resources.cameraTransform.translate;
	} else {
		viewProj = QFE::FRAMEWORK::UpdateMainCamera(sceneManager, &resources.cameraPosition);
		// ゲームビューの部屋プレビューは描画行列だけを変更する。
		windowManager_.TryGetGameViewerRoomPreview(viewProj, resources.cameraPosition);
	}
	windowManager_.SetAssetDropViewProjection(viewProj);
}

void GameEditor::Draw(QFE::FRAMEWORK::WindowsEngineResources& resources) {
	windowManager_.Draw(commandList_, resources);

	if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z)) {
		commandExecutor_.Undo();
	}
	if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y)) {
		commandExecutor_.Redo();
	}
	commandExecutor_.ExecuteCommand(&commandList_);
}

bool GameEditor::ConsumeGameModuleBuildAndReloadRequest() {
	return windowManager_.ConsumeGameModuleBuildAndReloadRequest();
}

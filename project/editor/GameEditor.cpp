#include "GameEditor.h"
#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"
#include "framework/input/InputFrameWork.h"

using namespace QFE::EDITOR;

void QFE::EDITOR::GameEditor::Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow) {
	windowManager_.Initialize(sceneManager, sceneTextureId, mainWindow);
	commandExecutor_.Initialize();
	commandList_.ClearCommands();

	activeCameraType_ = EditorCameraType::DebugCamera;
}

void QFE::EDITOR::GameEditor::Update(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources) {

	SCENE::SceneManager& sceneManager = *systems.sceneManager;

	commandList_.ClearCommands();

	// シーンビューがアクティブになった場合デバッグカメラに切り替える
	if (windowManager_.IsWindowFocused(EditorWindowType::SceneViewer)) {
		activeCameraType_ = EditorCameraType::DebugCamera;
	}
	// ゲームビューがアクティブになった場合メインカメラに切り替える
	if (windowManager_.IsWindowFocused(EditorWindowType::GameViewer)) {
		activeCameraType_ = EditorCameraType::MainCamera;
	}

	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();
	// デバッグカメラ操作処理
	if (windowManager_.IsWindowFocused(EditorWindowType::SceneViewer)) {
		float moveSpeed = 0.3f;

		// 移動処理
		if (inputInterface->GetKeyPress("Up")) {
			QFE::MATH::Vector3 forward = { 0.0f, 0.0f, 1.0f };
			QFE::MATH::Vector3 rotatedForward = QFE::MATH::TransformForwardDirection(resources.cameraTransform);
			resources.cameraTransform.translate += rotatedForward * moveSpeed;
		}
		if (inputInterface->GetKeyPress("Down")) {
			QFE::MATH::Vector3 forward = { 0.0f, 0.0f, 1.0f };
			QFE::MATH::Vector3 rotatedForward = QFE::MATH::TransformForwardDirection(resources.cameraTransform);
			resources.cameraTransform.translate -= rotatedForward * moveSpeed;
		}
		if (inputInterface->GetKeyPress("Left")) {
			QFE::MATH::Vector3 right = QFE::MATH::TransformRightDirection(resources.cameraTransform);
			resources.cameraTransform.translate -= right * moveSpeed;
		}
		if (inputInterface->GetKeyPress("Right")) {
			QFE::MATH::Vector3 right = QFE::MATH::TransformRightDirection(resources.cameraTransform);
			resources.cameraTransform.translate += right * moveSpeed;
		}

		// 回転処理
		if(inputInterface->GetMousePress(1)) {
			QFE::MATH::Vector2 mouseMove = inputInterface->GetMouseMove();
			resources.cameraTransform.rotate.y += mouseMove.x * 0.02f;
			resources.cameraTransform.rotate.x += mouseMove.y * 0.02f;
		}
	}

	// カメラのビュー行列と投影行列を取得
	QFE::MATH::Matrix4x4& viewProj = resources.viewProj;
	QFE::MATH::Vector3 currentCameraPos = { 0.0f, 0.0f, 0.0f };
	if (activeCameraType_ == QFE::EDITOR::EditorCameraType::DebugCamera) {
		QFE::MATH::Matrix4x4 viewMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(resources.cameraTransform).Inverse();
		QFE::MATH::Matrix4x4 projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
			3.14159f / 4.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
		viewProj = QFE::MATH::Matrix4x4::Multiply(viewMatrix, projectionMatrix);
		currentCameraPos = resources.cameraTransform.translate;
	} else {
		viewProj = QFE::FRAMEWORK::UpdateMainCamera(sceneManager);
	}
}

void GameEditor::Draw() {
	windowManager_.Draw(commandList_);
	commandExecutor_.ExecuteCommand(&commandList_);
}
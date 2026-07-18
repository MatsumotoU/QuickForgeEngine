#include "D3D12EngineUtility.h"
#include "D3D12EngineSystems.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/input/InputFrameWork.h"
#include "framework/scene/SceneFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"

#include "EngineDefines.h"

bool QFE::FRAMEWORK::SetupEngineSystems(
	D3D12EngineSystems* engineSystems, const QFE::FRAMEWORK::QuickForgeSetupConfig& config) {

	// 1. ウィンドウマネージャの作成
	engineSystems->gameWindowManager_ = CreateWindowManager();
	HWND hwnd = CreateWindowHandle(
		engineSystems->gameWindowManager_.get(),
		config.mainWindowName, config.mainWindowWidth, config.mainWindowHeight);
	// 成否確認
	if (engineSystems->gameWindowManager_ == nullptr || hwnd == nullptr) {
		QFE_LOG("Failed to create window manager or window handle.");
		return false;
	}

	// 2. グラフィックエンジンの作成
	engineSystems->graphicEngine_ =
		QFE::FRAMEWORK::CreateGraphicEngine(hwnd);
	// 成否確認
	if (engineSystems->graphicEngine_ == nullptr) {
		QFE_LOG("Failed to create graphic engine.");
		return false;
	}

	// 3. 入力インターフェースの作成
	engineSystems->inputInterface_ =
		QFE::FRAMEWORK::CreateInputInterface(hwnd, config.hInstance);
	// 成否確認
	if (engineSystems->inputInterface_ == nullptr) {
		QFE_LOG("Failed to create input interface.");
		return false;
	}

	// 4. シーンマネージャの作成
	engineSystems->sceneManager_ =
		QFE::FRAMEWORK::CreateSceneManager();
	// 成否確認
	if (engineSystems->sceneManager_ == nullptr) {
		QFE_LOG("Failed to create scene manager.");
		return false;
	}

	return true;
}

bool QFE::FRAMEWORK::ShutdownEngineSystems(D3D12EngineSystems* engineSystems) {
	// 1. 存在確認
	if (engineSystems == nullptr) {
		QFE_LOG("engineSystems is null.");
		return false;
	}

	// 2. グラフィックエンジンのシャットダウン
	if (engineSystems->graphicEngine_ != nullptr) {
		ShutdownGraphicEngine(engineSystems->graphicEngine_.get());
		engineSystems->graphicEngine_.reset();
	}
	// 3. 入力インターフェースのシャットダウン
	if (engineSystems->inputInterface_ != nullptr) {
		engineSystems->inputInterface_.reset();
	}
	// 4. シーンマネージャのシャットダウン
	if (engineSystems->sceneManager_ != nullptr) {
		engineSystems->sceneManager_.reset();
	}
	// 5. ウィンドウマネージャのシャットダウン
	if (engineSystems->gameWindowManager_ != nullptr) {
		engineSystems->gameWindowManager_.reset();
	}

	return true;
}

bool QFE::FRAMEWORK::FrameStart(D3D12EngineSystems* engineSystems) {
	QFE::FRAMEWORK::UpdateInputInterface(engineSystems->inputInterface_.get());
	return true;
}

bool QFE::FRAMEWORK::FrameEnd(D3D12EngineSystems* engineSystems) {
	QFE::FRAMEWORK::EndFrameSceneManager(engineSystems->sceneManager_.get());
	QFE::FRAMEWORK::EndFrameInputInterface(engineSystems->inputInterface_.get());
	return true;
}

bool QFE::FRAMEWORK::PreDraw(D3D12EngineSystems* engineSystems) {
	bool result = false;
	// カメラの更新
	result = QFE::FRAMEWORK::UpdateCurrentCamera(engineSystems->sceneManager_.get(), engineSystems->graphicEngine_.get());
	if (result == false) {
		QFE_LOG("Failed to update current camera.");
		return false;
	}

	// 描画前の処理
	result = QFE::FRAMEWORK::PreDrawGraphicEngine(engineSystems->graphicEngine_.get());
	if (result == false) {
		QFE_LOG("Failed to pre draw graphic engine.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::PostDraw(D3D12EngineSystems* engineSystems) {
	bool result = false;

	result = QFE::FRAMEWORK::PostDrawGraphicEngine(engineSystems->graphicEngine_.get());
	if(result == false) {
		QFE_LOG("Failed to post draw graphic engine.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::UpdateCamera(D3D12EngineSystems* engineSystems) {
	// カメラのビュー行列と投影行列を取得
	QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	entityManager.Each<QFE::SCENE::CameraComponent>([&](uint32_t entityId, QFE::SCENE::CameraComponent& cameraComp) {
		if (cameraComp.isMainCamera) {
			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				QFE::MATH::EulerTransform& cameraTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				cameraComp.viewMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(cameraTransform).Inverse();
				if (cameraComp.top_ - cameraComp.bottom_ != 0.0f) {
					cameraComp.aspectRatio_ = fabsf((cameraComp.right_ - cameraComp.left_) / (cameraComp.top_ - cameraComp.bottom_));
				} else {
					cameraComp.aspectRatio_ = 1.0f; // デフォルトのアスペクト比
				}
				cameraComp.projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
					cameraComp.fovY_, cameraComp.aspectRatio_, cameraComp.nearZ_, cameraComp.farZ_);

				viewProj = QFE::MATH::Matrix4x4::Multiply(cameraComp.viewMatrix, cameraComp.projectionMatrix);
			}
		}
		});
}

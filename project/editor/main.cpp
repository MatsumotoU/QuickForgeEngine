#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsEngineResources.h"

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/input/InputFrameWork.h"

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"

#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/loger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

#include "GameEditor.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	engineResources.windowName = "ShootingGameRuntime";
	engineResources.windowWidth = 1280;
	engineResources.windowHeight = 720;
	// QuickForgeエンジンのシステムを生成
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems engineSystems;
	if (!QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
		hInstance,
		engineResources.windowName,
		engineResources.windowWidth, engineResources.windowHeight,
		engineSystems)) {
		return -1;
	}
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& gameWindowManager = engineSystems.windowManager;
	auto& graphicEngine = engineSystems.graphicEngine;
	auto& guiManager = engineSystems.guiManager;
	auto& inputInterface = engineSystems.inputInterface;
	auto& fpsCounter = engineSystems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// エディタ用のシーンテクスチャ（レンダーターゲット）の作成
	QFE::GRAPHIC::RenderTargetHandle sceneRenderTargetHandle;
	QFE::FRAMEWORK::CreateOffScreenRenderTarget(
		graphicEngine.get(), sceneRenderTargetHandle, 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM);
	QFE::GRAPHIC::DirectXResourceHandle sceneTextureHandle;
	QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), sceneRenderTargetHandle, sceneTextureHandle);

	// レイトレーシング結果をEditorのシーンテクスチャへ出力する。
	engineResources.finalRenderTargetHandle = sceneRenderTargetHandle;

	// 初回フレームでもImGuiからSRVとして参照できる状態にしておく。
	QFE::FRAMEWORK::TransitionResourceToState(
		graphicEngine.get(), sceneTextureHandle, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_GPU_DESCRIPTOR_HANDLE sceneTextureGPUHandle = 
		graphicEngine->GetDirectXResourceContainer()->GetDescriptorHandleGPU(sceneTextureHandle, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
	ImTextureID sceneTextureId = static_cast<ImTextureID>(static_cast<uintptr_t>(sceneTextureGPUHandle.ptr));

	QFE::EDITOR::GameEditor gameEditor;
	gameEditor.Initialize(&sceneManager, sceneTextureId, engineSystems.windowManager->GetWindow(engineResources.windowName));

	// メインループ
	while (QFE::FRAMEWORK::IsMainWindowActive(engineSystems.windowManager.get())) {
		if (!QFE::FRAMEWORK::ProcessWindowsApplicationMessage()) {
			break;
		}
		if (!QFE::FRAMEWORK::BeginWindowsEngineFrame(engineSystems)) {
			break;
		}

		// エディタの更新処理
		gameEditor.Update(engineSystems, engineResources);

		// エンジンの描画前処理（フレーム開始時の共通処理）
		QFE::FRAMEWORK::EnginePreDraw(engineSystems, engineResources);

		// エディタの描画処理
		gameEditor.Draw();

		// エンジンの描画後処理（フレーム終了時の共通処理）
		QFE::FRAMEWORK::EnginePostDraw(engineSystems, engineResources);
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems);
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}

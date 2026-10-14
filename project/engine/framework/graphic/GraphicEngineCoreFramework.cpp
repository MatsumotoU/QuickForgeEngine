/// @file GraphicEngineCoreFramework.cpp
/// @brief Windows用のDirectX12グラフィックエンジンを生成する関数の実装ファイル
#include "GraphicEngineCoreFramework.h"

#include "EngineDefines.h"
#include "graphics/D3D12GraphicEngine.h"

#include "window/GameWindowManager.h"

std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(
	const QFE::WINDOW::IGameWindowManager& windowManager, const QFE::FRAMEWORK::GraphicEngineFeatureState& featureState) {

	// サポートOSに応じた内部グラフィックAPIの種類を取得
	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	// ウィンドウマネージャがnullptrの場合はエラーを報告
	if (gameWindowManager == nullptr) {
		QFE_LOG("Error: DirectX 12 requires a Windows game window manager.");
		return nullptr;
	}

	// メインウィンドウのハンドルを取得
	const HWND mainWindow = gameWindowManager->GetMainWindow();
	// メインウィンドウがnullptrの場合はエラーを報告
	if (mainWindow == nullptr) {
		QFE_LOG("Error: The window manager does not contain a main window.");
		return nullptr;
	}

	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine;
	// DirectX12グラフィックエンジンの生成
	graphicEngine = std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(
		mainWindow);
	if (graphicEngine == nullptr) {
		QFE_LOG("Error: Failed to create DirectX 12 graphic engine.");
		return nullptr;
	}
	// DirectX12グラフィックエンジンの初期化
	QFE::GRAPHIC::D3D12GraphicEngine* d3d12GraphicEngine =
		dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine.get());
	if (d3d12GraphicEngine == nullptr) {
		QFE_LOG("Error: Failed to cast to DirectX 12 graphic engine.");
		return nullptr;
	}

	// DirectX12グラフィックエンジンのコアシステムをセットアップ
	if (!d3d12GraphicEngine->SetUpCoreSystem()) {
		QFE_LOG("Error: Failed to set up the DirectX 12 core system.");
		return nullptr;
	}
	// 各機能のセットアップ
	if (featureState.isTextureLoaderEnabled && !d3d12GraphicEngine->SetUpTextureLoader()) {
		QFE_LOG("Warning: The texture loader is unavailable.");
	}
	if (featureState.isComputeEnabled && !d3d12GraphicEngine->SetUpComputePipeline()) {
		QFE_LOG("Warning: The compute pipeline is unavailable.");
	}
	if (featureState.isRaytracingEnabled && !d3d12GraphicEngine->SetUpRayTracingPipeline()) {
		QFE_LOG("Warning: The ray tracing pipeline is unavailable.");
	}
	if (featureState.isBuildInPipelineStateObjectsEnabled &&
		!d3d12GraphicEngine->SetUpBuildInPipelineStateObjects()) {
		QFE_LOG("Warning: The built-in pipeline state objects are unavailable.");
	}
	if (featureState.isLineRenderingEnabled && !d3d12GraphicEngine->SetUpLineRenderingPipeline()) {
		QFE_LOG("Warning: The line rendering pipeline is unavailable.");
	}

	// DirectX12グラフィックエンジンを生成
	return graphicEngine;
}

bool QFE::FRAMEWORK::IsGraphicEngineFeatureAvailable(
	const QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	QFE::FRAMEWORK::GraphicEngineFeature feature) {
	// グラフィックエンジンがnullptrの場合は利用不可とする
	if (graphicEngine == nullptr) {
		return false;
	}
	// 各機能の利用可能性を確認
	switch (feature) {
	case GraphicEngineFeature::CoreSystem:
		return graphicEngine->IsCoreSystemAvailable();
	case GraphicEngineFeature::TextureLoader:
		return graphicEngine->IsTextureLoaderAvailable();
	case GraphicEngineFeature::ComputePipeline:
		return graphicEngine->IsComputePipelineAvailable();
	case GraphicEngineFeature::RayTracingPipeline:
		return graphicEngine->IsRayTracingPipelineAvailable();
	case GraphicEngineFeature::BuiltInPipelineStateObjects:
		return graphicEngine->AreBuiltInPipelineStateObjectsAvailable();
	case GraphicEngineFeature::LineRendering:
		return graphicEngine->IsLineRenderingAvailable();
	default:
		return false;
	}
}

bool QFE::FRAMEWORK::PreDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine) {
	// グラフィックエンジンがnullptrの場合は利用不可とする
	auto* d3d12GraphicEngine = dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
	if (d3d12GraphicEngine == nullptr || !d3d12GraphicEngine->IsCoreSystemAvailable()) {
		return false;
	}
	// 描画前の処理を行う
	d3d12GraphicEngine->PreDraw();
	return true;
}

bool QFE::FRAMEWORK::PostDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine) {
	// グラフィックエンジンがnullptrの場合は利用不可とする
	auto* d3d12GraphicEngine = dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
	if (d3d12GraphicEngine == nullptr || !d3d12GraphicEngine->IsCoreSystemAvailable()) {
		return false;
	}
	// 描画後の処理を行う
	d3d12GraphicEngine->PostDraw();
	return true;
}

bool QFE::FRAMEWORK::ShutdownGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine) {
	// グラフィックエンジンがnullptrの場合は利用不可とする
	auto* d3d12GraphicEngine = dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
	if (d3d12GraphicEngine == nullptr) {
		return false;
	}
	// グラフィックエンジンの終了処理を行う
	d3d12GraphicEngine->Shutdown();
	return true;
}

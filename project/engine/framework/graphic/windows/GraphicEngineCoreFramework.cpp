/// @file GraphicEngineCoreFramework.cpp
/// @brief Windows用のDirectX12グラフィックエンジンを生成する関数の実装ファイル
#include "framework/graphic/GraphicEngineCoreFramework.h"

#include "EngineDefines.h"
#include "graphics/D3D12GraphicEngine.h"
#include "graphics/interface/DirectXRenderFramework.h"
#include "window/GameWindowManager.h"

std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(
	const QFE::WINDOW::IGameWindowManager& windowManager, const QFE::FRAMEWORK::GraphicEngineFeatureState& featureState) {

	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_LOG("Error: DirectX 12 requires a Windows game window manager.");
		return nullptr;
	}
	const HWND mainWindow = gameWindowManager->GetMainWindow();
	if (mainWindow == nullptr) {
		QFE_LOG("Error: The window manager does not contain a main window.");
		return nullptr;
	}

	auto graphicEngine = std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(mainWindow);
	if (!graphicEngine->SetUpCoreSystem()) {
		QFE_LOG("Error: Failed to set up the DirectX 12 core system.");
		return nullptr;
	}
	if (featureState.isTextureLoaderEnabled && !graphicEngine->SetUpTextureLoader()) {
		QFE_LOG("Warning: The texture loader is unavailable.");
	}
	if (featureState.isComputeEnabled && !graphicEngine->SetUpComputePipeline()) {
		QFE_LOG("Warning: The compute pipeline is unavailable.");
	}
	if (featureState.isRaytracingEnabled && !graphicEngine->SetUpRayTracingPipeline()) {
		QFE_LOG("Warning: The ray tracing pipeline is unavailable.");
	}
	if (featureState.isBuildInPipelineStateObjectsEnabled &&
		!graphicEngine->SetUpBuildInPipelineStateObjects()) {
		QFE_LOG("Warning: The built-in pipeline state objects are unavailable.");
	}
	if (featureState.isLineRenderingEnabled && !graphicEngine->SetUpLineRenderingPipeline()) {
		QFE_LOG("Warning: The line rendering pipeline is unavailable.");
	}
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

bool QFE::FRAMEWORK::SetSwapChainRenderTarget(
	QFE::GRAPHIC::IGraphicEngine& graphicEngine) {
	return QFE::GRAPHIC::SetRenderTarget(
		&graphicEngine,
		{ QFE::GRAPHIC::RenderTargetHandle::SwapChain });
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

#include "framework/gui/GuiCoreFramework.h"

#include "framework/window/WindowCoreFrameWork.h"
#include "engine/window/GameWindowManager.h"

#include "framework/graphic/GraphicContext.h"
#include "gui/D3D12GuiManager.h"

#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/vram/descriptors/DescriptorHeapManager.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"

std::unique_ptr<QFE::GUI::IGuiManager> QFE::FRAMEWORK::CreateGuiManager(
	const QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const QFE::WINDOW::IGameWindowManager* windowManager,
	const std::string& windowName) {

	// Windowsのウィンドウマネージャにキャスト
	const QFE::WINDOW::GameWindowManager* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(windowManager);
	// ウィンドウマネージャがnullptrの場合はエラーを報告
	if(gameWindowManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid window manager type", SystemError::Abort);
		return nullptr;
	}

	// ウィンドウハンドルの取得
	HWND hwnd = nullptr;
	if(windowName.empty()) {
		hwnd = gameWindowManager->GetMainWindow();
	} else {
		hwnd = gameWindowManager->GetWindow(windowName);
	}
	// ウィンドウハンドルが取得できなかった場合はエラーを報告
	if(hwnd == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Window not found - " + windowName, SystemError::Abort);
		return nullptr;
	}

	// GraphicEngineをDirectXGraphicEngineにキャスト
	const QFE::GRAPHIC::D3D12GraphicEngine* d3d12GraphicEngine =
		dynamic_cast<const QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
	// GraphicEngineがnullptrの場合はエラーを報告
	if(d3d12GraphicEngine == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid graphic engine type", SystemError::Abort);
		return nullptr;
	}

	// 必要な機能の取得
	QFE::GRAPHIC::DirectXDevice* device = d3d12GraphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = d3d12GraphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::DescriptorHeapManager* descriptorHeapManager = d3d12GraphicEngine->GetDescriptorHeapManager();
	QFE::GRAPHIC::RenderPass* renderPass = d3d12GraphicEngine->GetRenderPass();

	// SRVヒープの空のハンドルを作成
	QFE::GRAPHIC::DescriptorHandles emptySrvHandles = 
		descriptorHeapManager->CreateEmptyHeapHandle(QFE::GRAPHIC::DescriptorHeapType::SRV);

	// GUIマネージャの初期化
	QFE::GUI::D3D12GuiManager guiManager;
	QFE::GUI::D3D12GuiManagerInitDesc guiInitDesc;
	guiInitDesc.device = device->GetDevice();
	guiInitDesc.commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	guiInitDesc.bufferCount = renderPass->GetSwapChainBufferCount();
	guiInitDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	guiInitDesc.srvHeap = descriptorHeapManager->GetDescriptorHeap(QFE::GRAPHIC::DescriptorHeapType::SRV);
	guiInitDesc.cpuHandle = emptySrvHandles.cpuHandle_;
	guiInitDesc.gpuHandle = emptySrvHandles.gpuHandle_;
	guiInitDesc.hwnd = hwnd;
	guiManager.Initialize(guiInitDesc);

	return std::make_unique<QFE::GUI::D3D12GuiManager>(std::move(guiManager));
}

bool ShutdownGui(QFE::GUI::IGuiManager* guiManager)
{
	// GUIマネージャがnullptrの場合はエラーを報告
	if(guiManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("ShutdownGui: Invalid gui manager", SystemError::Abort);
		return false;
	}
	// D3D12GuiManagerにキャストして終了処理を呼び出す
	if(QFE::GUI::D3D12GuiManager* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(guiManager)) {
		manager->Shutdown();
		return true;
	}
	return false;
}
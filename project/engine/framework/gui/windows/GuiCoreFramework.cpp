#include "framework/gui/GuiCoreFramework.h"

#include "EngineDefines.h"
#include "window/GameWindowManager.h"
#include "gui/D3D12GuiManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/vram/descriptors/DescriptorHeapManager.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"

std::unique_ptr<QFE::GUI::IGuiManager> QFE::FRAMEWORK::CreateGuiManager(
	const QFE::GRAPHIC::IGraphicEngine& graphicEngine,
	const QFE::WINDOW::IGameWindowManager& windowManager,
	const std::string& windowName) {

	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid window manager type", SystemError::Abort);
		return nullptr;
	}

	const HWND hwnd = windowName.empty()
		? gameWindowManager->GetMainWindow()
		: gameWindowManager->GetWindow(windowName);
	if (hwnd == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Window not found - " + windowName, SystemError::Abort);
		return nullptr;
	}

	const auto* d3d12GraphicEngine =
		dynamic_cast<const QFE::GRAPHIC::D3D12GraphicEngine*>(&graphicEngine);
	if (d3d12GraphicEngine == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid graphic engine type", SystemError::Abort);
		return nullptr;
	}

	QFE::GRAPHIC::DirectXDevice* device = d3d12GraphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXCommandManager* commandManager =
		d3d12GraphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::DescriptorHeapManager* descriptorHeapManager =
		d3d12GraphicEngine->GetDescriptorHeapManager();
	QFE::GRAPHIC::RenderPass* renderPass = d3d12GraphicEngine->GetRenderPass();
	QFE::GRAPHIC::DescriptorHandles emptySrvHandles =
		descriptorHeapManager->CreateEmptyHeapHandle(QFE::GRAPHIC::DescriptorHeapType::SRV);

	QFE::GUI::D3D12GuiManagerInitDesc guiInitDesc;
	guiInitDesc.device = device->GetDevice();
	guiInitDesc.commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	guiInitDesc.bufferCount = renderPass->GetSwapChainBufferCount();
	guiInitDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	guiInitDesc.srvHeap = descriptorHeapManager->GetDescriptorHeap(
		QFE::GRAPHIC::DescriptorHeapType::SRV);
	guiInitDesc.cpuHandle = emptySrvHandles.cpuHandle_;
	guiInitDesc.gpuHandle = emptySrvHandles.gpuHandle_;
	guiInitDesc.hwnd = hwnd;

	auto guiManager = std::make_unique<QFE::GUI::D3D12GuiManager>();
	guiManager->Initialize(guiInitDesc);
	return guiManager;
}

void QFE::FRAMEWORK::PreDrawGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->PreDraw();
	}
}

void QFE::FRAMEWORK::PostDrawGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->PostDraw();
	}
}

bool QFE::FRAMEWORK::ShutdownGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->Shutdown();
		return true;
	}
	return false;
}

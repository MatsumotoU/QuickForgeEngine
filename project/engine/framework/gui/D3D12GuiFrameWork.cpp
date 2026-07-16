#include "D3D12GuiFrameWork.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"

#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/vram/descriptors/DescriptorHeapManager.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"

std::unique_ptr<QFE::GUI::D3D12GuiManager> QFE::FRAMEWORK::CreateGuiManager(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd) {
	// 必要な機能の取得
	QFE::GRAPHIC::DirectXDevice* device = graphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::DescriptorHeapManager* descriptorHeapManager = graphicEngine->GetDescriptorHeapManager();
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();

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
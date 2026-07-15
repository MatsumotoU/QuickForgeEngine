#include "D3D12GuiFrameWork.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"

std::unique_ptr<QFE::GUI::D3D12GuiManager> QFE::FRAMEWORK::CreateGuiManager(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd) {
	// GUIマネージャの初期化
	QFE::GUI::D3D12GuiManager guiManager;
	QFE::GUI::D3D12GuiManagerInitDesc guiInitDesc;
	guiInitDesc.device = graphicEngine->GetDevice();
	guiInitDesc.commandList = graphicEngine->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	guiInitDesc.bufferCount = graphicEngine->GetSwapChainBufferCount();
	guiInitDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	guiInitDesc.srvHeap = graphicEngine->GetSRVDescriptorHeap();
	QFE::GRAPHIC::DescriptorHandles srvHandles = graphicEngine->CreateExternalSRVDescriptor();
	guiInitDesc.cpuHandle = srvHandles.cpuHandle_;
	guiInitDesc.gpuHandle = srvHandles.gpuHandle_;
	guiInitDesc.hwnd = hwnd;
	guiManager.Initialize(guiInitDesc);

	return std::make_unique<QFE::GUI::D3D12GuiManager>(std::move(guiManager));
}
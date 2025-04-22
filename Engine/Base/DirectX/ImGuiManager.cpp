#include "ImGuiManager.h"
#include "../Windows/WinApp.h"
#include "DirectXCommon.h"

#ifdef DEBUG
#include "../MyDebugLog.h"
#endif // DEBUG

ImGuiManager::ImGuiManager() {
	stateCheck_ = 0;
#ifdef _DEBUG
	debugLog_ = MyDebugLog::GetInstatnce();
	debugLog_->Log("ImGuiManager : Generate Instance");
#endif // DEBUG
}

ImGuiManager::~ImGuiManager() {
#ifdef _DEBUG
	if (stateCheck_ != 0) {
		debugLog_->Log(std::format("!!! ImGuiManager : Error{} !!!\n", stateCheck_));
	}
#endif // _DEBUG
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
#ifdef _DEBUG
	stateCheck_++;
#endif // _DEBUG

	device_ = dxCommon->GetDevice();
	srvDescriptorHeap_ = CreateDescriptorHeap(device_,D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,128,true);
	commandList_ = dxCommon->GetCommandList();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	// ここでsrvDescriptorHeapの先頭にviewを作ってる
	ImGui_ImplDX12_Init(device_.Get(),
		dxCommon->GetSwapChainDesc()->BufferCount,
		dxCommon->GetRtvDesc()->Format,
		srvDescriptorHeap_.Get(),
		srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::EndImGui() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#ifdef _DEBUG
	stateCheck_--;
#endif // _DEBUG
}

void ImGuiManager::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// DescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap_.Get()};
	commandList_.Get()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void ImGuiManager::EndFrame() {

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_.Get());
}

ID3D12DescriptorHeap* ImGuiManager::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_.Get();
}

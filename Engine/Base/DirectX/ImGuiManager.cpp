#include "ImGuiManager.h"
#include "../Windows/WinApp.h"
#include "DirectXCommon.h"

#ifdef DEBUG
#include "../MyDebugLog.h"
#endif // DEBUG

ImGuiManager::ImGuiManager() {
	
#ifdef _DEBUG
	stateCheck_ = 0;
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

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon, ID3D12DescriptorHeap* srvDescriptorHeap) {
#ifdef _DEBUG
	stateCheck_++;
#endif // _DEBUG

	device_ = dxCommon->GetDevice();
	srvDescriptorHeap_ = srvDescriptorHeap;//CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
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
		srvDescriptorHeap_,
		srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// ImPlotの初期化
	ImPlot::CreateContext();
}

void ImGuiManager::EndImGui() {
	// ImPilotの終了処理
	ImPlot::DestroyContext();

	// ImGuiの終了処理
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
	ImGuizmo::BeginFrame();

	// DescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap_};
	commandList_.Get()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void ImGuiManager::EndFrame() {

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_.Get());
}

ID3D12DescriptorHeap* ImGuiManager::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_;
}

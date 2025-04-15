#include "ImGuiManager.h"
#include "../Windows/WinApp.h"
#include "DirectXCommon.h"

ImGuiManager* ImGuiManager::GetInstatnce() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	device_ = dxCommon->GetDevice();
	srvDescriptorHeap_ = dxCommon->CreateDescriptorHeap(device_,D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,128,true);
	commandList_ = dxCommon->GetCommandList();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(device_,
		dxCommon->GetSwapChainDesc()->BufferCount,
		dxCommon->GetRtvDesc()->Format,
		srvDescriptorHeap_,
		srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::EndImGui() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// DescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap_ };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
}

void ImGuiManager::EndFrame() {
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
}

ID3D12DescriptorHeap* ImGuiManager::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_;
}

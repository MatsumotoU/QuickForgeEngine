#include "ImGuiFlameController.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // DEBUG
#include <cassert>

ImGuiFlameController::ImGuiFlameController() {
#ifdef _DEBUG
	stateCheck_ = 0;
	DebugLog("ImGuiManager : Generate Instance");
#endif // DEBUG
}

ImGuiFlameController::~ImGuiFlameController() {
#ifdef _DEBUG
	if (stateCheck_ != 0) {
		DebugLog(std::format("!!! ImGuiManager : Error{} !!!\n", stateCheck_));
	}
#endif // _DEBUG
}

void ImGuiFlameController::Initialize(const HWND& hwnd, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* const* srvDescriptorHeap) {
#ifdef _DEBUG
	stateCheck_++;
#endif // _DEBUG
	commandList_ = commandList;
	assert(commandList_ != nullptr);
	srvDescriptorHeap_ = srvDescriptorHeap;
	assert(srvDescriptorHeap_ != nullptr);

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui_ImplWin32_Init(hwnd);

	// ImPlotの初期化
	ImPlot::CreateContext();
}

void ImGuiFlameController::EndImGui() {
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

void ImGuiFlameController::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// DescriptorHeapの設定
	commandList_->SetDescriptorHeaps(1, srvDescriptorHeap_);
}

void ImGuiFlameController::EndFrame() {
	// ImGuiの描画
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
}
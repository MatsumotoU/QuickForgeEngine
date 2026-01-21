#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // DEBUG
#include <cassert>

ImGuiFlameController::ImGuiFlameController() {
#ifdef QFE_OPTIMIZE_OFF
	stateCheck_ = 0;
	DebugLog("ImGuiManager : Generate Instance");
#endif // DEBUG
}

ImGuiFlameController::~ImGuiFlameController() {
#ifdef QFE_OPTIMIZE_OFF
	if (stateCheck_ != 0) {
		DebugLog(std::format("!!! ImGuiManager : Error{} !!!\n", stateCheck_));
	}
#endif // QFE_OPTIMIZE_OFF
}

void ImGuiFlameController::Initialize(const HWND& hwnd, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* const* srvDescriptorHeap) {
#ifdef QFE_OPTIMIZE_OFF
	stateCheck_++;
#endif // QFE_OPTIMIZE_OFF
	commandList_ = commandList;
	assert(commandList_ != nullptr);
	srvDescriptorHeap_ = srvDescriptorHeap;
	assert(srvDescriptorHeap_ != nullptr);

	// ImGui縺ｮ蛻晄悄蛹・
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui_ImplWin32_Init(hwnd);

	// ImPlot縺ｮ蛻晄悄蛹・
	ImPlot::CreateContext();
}

void ImGuiFlameController::EndImGui() {
	// ImPilot縺ｮ邨ゆｺ・・逅・
	ImPlot::DestroyContext();

	// ImGui縺ｮ邨ゆｺ・・逅・
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#ifdef QFE_OPTIMIZE_OFF
	stateCheck_--;
#endif // QFE_OPTIMIZE_OFF
}

void ImGuiFlameController::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// DescriptorHeap縺ｮ險ｭ螳・
	commandList_->SetDescriptorHeaps(1, srvDescriptorHeap_);
}

void ImGuiFlameController::EndFrame(D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferCpuHandle) {
	// ImGui縺ｮ謠冗判
	ImGui::Render();

	// 霑ｽ蜉: 繝薙Η繝ｼ繝昴・繝育畑縺ｮ繝励Λ繝・ヨ繝輔か繝ｼ繝繧ｦ繧｣繝ｳ繝峨え繧よ緒逕ｻ
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// 繝ｬ繝ｳ繝繝ｼ繧ｿ繝ｼ繧ｲ繝・ヨ縺ｮ險ｭ螳・
	commandList_->OMSetRenderTargets(1, &currentBackBufferCpuHandle, FALSE, nullptr);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
}

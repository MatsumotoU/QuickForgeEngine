#include "engine/include/utility/DebugTool/ImGui/ImGuiInitializer.h"
#include <cassert>
#include <filesystem>
using namespace QFE;
void ImGuiInitializer::Initialize(
	ID3D12Device* device, uint32_t bufferCount, DXGI_FORMAT rtvFormat,
	ID3D12DescriptorHeap* srvHeap, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {
	// ImGUi繝輔か繝ｳ繝医・險ｭ螳・
	ImFontConfig config = {};
	ImGuiIO& io = ImGui::GetIO();
	config.SizePixels = 12.0f;
	const char* fontPath = "C:/Windows/Fonts/YuGothB.ttc";
	if (std::filesystem::exists(fontPath)) {
		ImFont* font = io.Fonts->AddFontFromFileTTF(
			fontPath,
			config.SizePixels,
			&config,
			io.Fonts->GetGlyphRangesJapanese());

		if (font) {
			io.FontDefault = font;
			io.FontGlobalScale = 1.0f;
			io.Fonts->Build();
		}
	} else {
		OutputDebugStringA("Not Find Fonts: YuGothB.ttc\n");
	}

	// Viewport縺ｮ險ｭ螳・
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // 隍・焚繧ｦ繧｣繝ｳ繝峨え繧呈怏蜉ｹ蛹・

	// ImGui縺ｮ蛻晄悄蛹・
	bool isInit = ImGui_ImplDX12_Init(device,
		bufferCount,
		rtvFormat,
		srvHeap,
		cpuHandle,
		gpuHandle);
	assert(isInit);
	isInit;
}

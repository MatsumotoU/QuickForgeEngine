#include "D3D12GuiManager.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

#include <filesystem>

#include "EngineDefines.h"

using namespace QFE::GUI;

void D3D12GuiManager::Initialize(const D3D12GuiManagerInitDesc& desc) {
	// ImGuiのコンテキストを作成
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	commandList_ = desc.commandList;

	// フラグの有効化
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// ImGuiのバックエンドを初期化
	bool isInit = ImGui_ImplDX12_Init(desc.device,
		desc.bufferCount,
		desc.rtvFormat,
		desc.srvHeap,
		desc.cpuHandle,
		desc.gpuHandle);
	if (!isInit) {
		QFE_REPORT_SYSTEM_ERROR("ImGui_ImplDX12_Init Failed", SystemError::Abort);
	}

	// ImGuiのWin32バックエンドを初期化
	isInit = ImGui_ImplWin32_Init(desc.hwnd);
	if (!isInit) {
		QFE_REPORT_SYSTEM_ERROR("ImGui_ImplWin32_Init Failed", SystemError::Abort);
	}

	QFE_LOG("D3D12GuiManager initialized successfully.");
}

void D3D12GuiManager::Update() {
}

void D3D12GuiManager::PreDraw() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void D3D12GuiManager::PostDraw() {
	ImGui::Render();
	
	if(commandList_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("commandList_ is nullptr", SystemError::Abort);
	}
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
	
	ImGui::EndFrame();
}

void D3D12GuiManager::Shutdown() {
	// ImGuiのバックエンドを終了
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	QFE_LOG("D3D12GuiManager shutdown successfully.");
}
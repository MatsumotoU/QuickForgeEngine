#include "ImGuiManager.h"

ImGuiManager* ImGuiManager::GetInstatnce() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
}

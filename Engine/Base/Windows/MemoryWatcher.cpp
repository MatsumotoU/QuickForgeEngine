#include "MemoryWatcher.h"
#pragma comment(lib,"psapi.lib")

#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#include "Base/MyDebugLog.h"
#endif // _DEBUG

MemoryWatcher::MemoryWatcher() {
#ifdef _DEBUG
	DebugLog("GlobalMemoryStatus");

	MEMORYSTATUSEX memInfo{};
	memInfo.dwLength = sizeof(memInfo);
	if (GlobalMemoryStatusEx(&memInfo)) {
		// 総物理メモリ（バイト単位）
		size_t totalPhys = static_cast<size_t>(memInfo.ullTotalPhys);
		// 空き物理メモリ
		size_t availPhys = static_cast<size_t>(memInfo.ullAvailPhys);
		// 使用中の物理メモリ
		size_t usedPhys = totalPhys - availPhys;

		DebugLog(std::format("Total: {} GB", totalPhys / (1024.0f * 1024.0f * 1024.0f)));
		DebugLog(std::format("Used: {} GB", usedPhys / (1024.0f * 1024.0f * 1024.0f)));
		DebugLog(std::format("Free: {} GB", availPhys / (1024.0f * 1024.0f * 1024.0f)));
	}
#endif // _DEBUG
}

void MemoryWatcher::DrawImGui() {
	MEMORYSTATUSEX memInfo{};
	memInfo.dwLength = sizeof(memInfo);
	if (GlobalMemoryStatusEx(&memInfo)) {
#ifdef _DEBUG
		ImGui::Text("GlobalMemoryStatus");
#endif // _DEBUG
		
		// 総物理メモリ（バイト単位）
		size_t totalPhys = static_cast<size_t>(memInfo.ullTotalPhys);
		// 空き物理メモリ
		size_t availPhys = static_cast<size_t>(memInfo.ullAvailPhys);
		// 使用中の物理メモリ
		size_t usedPhys = totalPhys - availPhys;

		float usedRate = static_cast<float>(usedPhys) / static_cast<float>(totalPhys);
#ifdef _DEBUG
		ImGui::Text("Total: %.2f GB", totalPhys / (1024.0f * 1024.0f * 1024.0f));
		ImGui::Text("Used:  %.2f GB", usedPhys / (1024.0f * 1024.0f * 1024.0f));
		ImGui::Text("Free:  %.2f GB", availPhys / (1024.0f * 1024.0f * 1024.0f));
		ImGui::ProgressBar(usedRate, ImVec2(300, 0),
			std::format("{:.1f}%", usedRate * 100.0f).c_str());
#endif // _DEBUG

		PROCESS_MEMORY_COUNTERS pmc{};
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
			size_t workingSet = pmc.WorkingSetSize;
			size_t commitSize = pmc.PagefileUsage;
			float processRate = static_cast<float>(workingSet) / static_cast<float>(totalPhys);
#ifdef _DEBUG
			ImGui::Separator();
			ImGui::Text("LocalMemoryStatus");
			ImGui::Text("Process WorkingSet: %.2f MB", workingSet / (1024.0f * 1024.0f));
			ImGui::Text("Process CommitSize: %.2f MB", commitSize / (1024.0f * 1024.0f));
			ImGui::ProgressBar(processRate, ImVec2(300, 0),
				std::format("Process: {:.3f}%", processRate * 100.0f).c_str());
#endif // _DEBUG
			
		}
	}
}

size_t MemoryWatcher::GetProcessMemoryUsageMB() {
	PROCESS_MEMORY_COUNTERS pmc{};
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		return pmc.WorkingSetSize / (1024 * 1024);
	}
	return 0;
}

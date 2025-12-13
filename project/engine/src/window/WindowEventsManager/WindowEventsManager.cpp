#include "engine/include/window/windowEventsManager/WindowEventsManager.h"

#include "engine/include/window/windowEventsManager/eventSystems/DropFileEvent.h"
#include "engine/include/window/windowEventsManager/eventSystems/ExitAppEvent.h"
#include "engine/include/window/windowEventsManager/eventSystems/OnFocusEvent.h"

#ifdef _DEBUG
#include "engine/include/utility/String/HwndConvertString.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#include "Externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

WindowEventsManager::WindowEventsManager() {
	// 繧､繝吶Φ繝医す繧ｹ繝・Β縺ｮ蛻晄悄蛹・
	eventData_ = nlohmann::json::object();
	eventSystems_[0] = std::make_unique<DropFileEvent>(eventData_);
	eventSystems_[1] = std::make_unique<ExitAppEvent>(eventData_);
	eventSystems_[2] = std::make_unique<OnFocusEvent>(eventData_);
}

LRESULT CALLBACK WindowEventsManager::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// TODO: ImGui縺ｯImGui縺ｮ譁ｹ縺ｫ遘ｻ蜍輔＆縺帙ｋ
#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // _DEBUG

	// WM_NCCREATE譎ゅ・縺ｿCREATESTRUCT縺九ｉthis繧貞叙蠕励＠縺ｦ菫晏ｭ・
	if (msg == WM_NCCREATE) {
		CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
		auto* pThis = static_cast<WindowEventsManager*>(cs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	// 莉･髯阪・USERDATA縺九ｉ蜿門ｾ・
	auto* pThis = reinterpret_cast<WindowEventsManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (pThis) {
		return pThis->HandleMessage(hwnd, msg, wparam, lparam);
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT WindowEventsManager::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	for (const auto& eventSystem : eventSystems_) {
		if (eventSystem && msg == eventSystem->GetEventType()) {
#ifdef _DEBUG
			DebugLog("Call Event WindowName: " + ConvertString(HwndConvertString::HwndToString(hwnd)));
#endif
			eventSystem->OnEvent(wparam, lparam);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

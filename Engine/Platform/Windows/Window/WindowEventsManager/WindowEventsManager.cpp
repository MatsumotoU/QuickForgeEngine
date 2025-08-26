#include "WindowEventsManager.h"

#include "EventSystems/DropFile/DropFileEvent.h"
#include "EventSystems/ExitApp/ExitAppEvent.h"

#ifdef _DEBUG
#include "AppUtility/String/HwndConvertString.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
#include "Externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

WindowEventsManager::WindowEventsManager() {
	// イベントシステムの初期化
	eventData_ = nlohmann::json::object();
	eventSystems_[0] = std::make_unique<DropFileEvent>(eventData_);
	eventSystems_[1] = std::make_unique<ExitAppEvent>(eventData_);
}

LRESULT CALLBACK WindowEventsManager::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// WM_NCCREATE時のみCREATESTRUCTからthisを取得して保存
	if (msg == WM_NCCREATE) {
		CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
		auto* pThis = static_cast<WindowEventsManager*>(cs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	// 以降はUSERDATAから取得
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

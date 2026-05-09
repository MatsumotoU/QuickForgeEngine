#include "engine/include/window/windowEventsManager/WindowEventsManager.h"

#include "engine/include/window/windowEventsManager/eventSystems/DropFileEvent.h"
#include "engine/include/window/windowEventsManager/eventSystems/ExitAppEvent.h"
#include "engine/include/window/windowEventsManager/eventSystems/OnFocusEvent.h"

#include "engine/include/core/EngineDefines.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/String/HwndConvertString.h"
#include "engine/include/utility/String/MyString.h"
#include "Externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // QFE_OPTIMIZE_OFF

namespace QFE {

	WindowEventsManager::WindowEventsManager() {
		// イベントシステムの初期化
		eventData_ = nlohmann::json::object();
		eventSystems_[0] = std::make_unique<DropFileEvent>(eventData_);
		eventSystems_[1] = std::make_unique<ExitAppEvent>(eventData_);
		eventSystems_[2] = std::make_unique<OnFocusEvent>(eventData_);
	}

	LRESULT CALLBACK WindowEventsManager::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		// TODO: ImGuiの描画に影響が出力されるため要移動
#ifdef QFE_OPTIMIZE_OFF
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
			return true;
		}
#endif // QFE_OPTIMIZE_OFF

		// WM_NCCREATEメッセージ、CREATESTRUCTからThisポインタを取り出して保持
		if (msg == WM_NCCREATE) {
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
			auto* pThis = static_cast<WindowEventsManager*>(cs->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		// それ以外、USERDATAから取り出す
		auto* pThis = reinterpret_cast<WindowEventsManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (pThis) {
			return pThis->HandleMessage(hwnd, msg, wparam, lparam);
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	LRESULT WindowEventsManager::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		for (const auto& eventSystem : eventSystems_) {
			if (eventSystem && msg == eventSystem->GetEventType()) {
#ifdef QFE_OPTIMIZE_OFF
				QFE_LOG("Call Event WindowName: " + ConvertString(HwndConvertString::HwndToString(hwnd)));
#endif
				eventSystem->OnEvent(wparam, lparam);
				return 0;
			}
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

}

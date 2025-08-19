#include "ExitAppEvent.h"
#ifdef _DEBUG
#include "Utility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
#endif // _DEBUG

ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; // Unused parameter
	lparam; // Unused parameter
#ifdef _DEBUG
	DebugLog("Call ExitAppEvent");
#endif
	// アプリケーションの終了処理
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

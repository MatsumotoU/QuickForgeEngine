#include "engine/include/window/windowEventsManager/EventSystems/ExitAppEvent.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // QFE_OPTIMIZE_OFF
using namespace QFE;
ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; // Unused parameter
	lparam; // Unused parameter
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Call ExitAppEvent");
#endif
	// アプリケーション終了フラグを立てる
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

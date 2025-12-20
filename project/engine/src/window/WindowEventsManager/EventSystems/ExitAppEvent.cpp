#include "engine/include/window/windowEventsManager/EventSystems/ExitAppEvent.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // _DEBUG

ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; // Unused parameter
	lparam; // Unused parameter
#ifdef _DEBUG
	DebugLog("Call ExitAppEvent");
#endif
	// 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ邨ゆｺ・・逅・
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

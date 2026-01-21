#include "engine/include/window/windowEventsManager/EventSystems/OnFocusEvent.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // QFE_OPTIMIZE_OFF

OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Whindow Focused");
#endif // QFE_OPTIMIZE_OFF
	LuaScriptResourceManager::GetInstance()->ReloadAllScripts();
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}



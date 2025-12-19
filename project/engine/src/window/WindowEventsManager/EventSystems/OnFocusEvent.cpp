#include "engine/include/window/windowEventsManager/EventSystems/OnFocusEvent.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // _DEBUG

OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
#ifdef _DEBUG
	DebugLog("Whindow Focused");
#endif // _DEBUG
	LuaScriptResourceManager::GetInstance()->ReloadAllScripts();
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}

#include "OnFocusEvent.h"
#include "Assets/Script/LuaScriptResourceManager.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
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

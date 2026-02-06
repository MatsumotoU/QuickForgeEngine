#include "engine/include/window/windowEventsManager/EventSystems/OnFocusEvent.h"

#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/MyString.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Whindow Focused");
#endif // QFE_OPTIMIZE_OFF

	if (SceneManager::GetInstance()->GetLuaScriptExecutor() != nullptr) {
		SceneManager::GetInstance()->GetLuaScriptExecutor()->ReloadAllScripts();
	} else {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("LuaScriptExecutor is nullptr", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
	}
	
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}



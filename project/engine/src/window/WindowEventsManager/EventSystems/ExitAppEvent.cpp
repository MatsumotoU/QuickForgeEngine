#include "engine/include/window/windowEventsManager/EventSystems/ExitAppEvent.h"
#include "engine/include/core/EngineDefines.h"

using namespace QFE;
ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; // Unused parameter
	lparam; // Unused parameter
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Call ExitAppEvent");
#endif
	// アプリケーション終了フラグを立てる
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

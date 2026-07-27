#include "ExitAppEvent.h"
#include "EngineDefines.h"

using namespace QFE;
ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; // Unused parameter
	lparam; // Unused parameter
	QFE_LOG("Call ExitAppEvent");

	// アプリケーション終了フラグを立てる
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

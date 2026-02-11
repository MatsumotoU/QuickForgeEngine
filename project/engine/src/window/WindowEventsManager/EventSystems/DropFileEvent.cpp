#include "engine/include/window/windowEventsManager/EventSystems/DropFileEvent.h"
#include "engine/include/utility/String/MyString.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/assets/AssetManager.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF
using namespace QFE;
DropFileEvent::DropFileEvent(nlohmann::json& data):IEvent(data) {}

void DropFileEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	lparam;
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Call DropFileEvent");
#endif // QFE_OPTIMIZE_OFF

	// ドロップされたファイルのパスを取得
	HDROP hDrop = (HDROP)wparam;
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < fileCount; ++i) {
		wchar_t filePath[MAX_PATH];
		DragQueryFile(hDrop, i, filePath, MAX_PATH);
		// ファイルパスをイベントデータに格納
		eventData_["DropFilePath"] = ConvertString(filePath);
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Drop File: " + ConvertString(filePath));
#endif // QFE_OPTIMIZE_OFF
	}
	DragFinish(hDrop);
}

 UINT DropFileEvent::GetEventType() {
	return WM_DROPFILES;
}

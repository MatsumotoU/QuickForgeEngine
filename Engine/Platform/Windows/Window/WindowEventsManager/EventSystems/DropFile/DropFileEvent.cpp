#include "DropFileEvent.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
#endif // _DEBUG

DropFileEvent::DropFileEvent(nlohmann::json& data):IEvent(data) {}

void DropFileEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	lparam;
#ifdef _DEBUG
	DebugLog("Call DropFileEvent");
#endif // _DEBUG

	// ドロップされたファイルの処理
	HDROP hDrop = (HDROP)wparam;
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < fileCount; ++i) {
		wchar_t filePath[MAX_PATH];
		DragQueryFile(hDrop, i, filePath, MAX_PATH);
		eventData_["DropFilePath"] = ConvertString(filePath);
#ifdef _DEBUG
		DebugLog("Drop File: " + ConvertString(filePath));
#endif // _DEBUG
	}
	DragFinish(hDrop);
}

 UINT DropFileEvent::GetEventType() {
	return WM_DROPFILES;
}

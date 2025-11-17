#include "CsharpScriptBridge.h"
// サブシステム
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"

void LogFromCSharp(const char* message) {
	message;
	DebugLog(std::string("[C#] ") + message,LogLevel::EditorInfo, std::source_location::current());
}

#include "WinAppDebugCore.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"

#pragma comment(lib,"Dbghelp.lib")

WinAppDebugCore::WinAppDebugCore(const LPSTR& lpCmdLine) {
	SetUnhandledExceptionFilter(ExportDump);

	DebugLog("=====WinAppDebugCore=====");

	// exeを起動したパス
	wchar_t fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	std::string exeName(ConvertString(fileName));
	
	// コマンド引数確認
	if (std::strcmp(lpCmdLine, "\0") != 0) {
		DebugLog("!!! EnebleCommandLineArguments !!!");
		DebugLog(std::format("EnebleCommand : {}\n", lpCmdLine));
	} else {
		DebugLog("DisableCommandLineArguments");
	}

	DebugLog("");
}

WinAppDebugCore::~WinAppDebugCore() {
}

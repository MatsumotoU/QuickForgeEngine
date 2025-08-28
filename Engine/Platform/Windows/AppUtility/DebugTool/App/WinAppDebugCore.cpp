#include "WinAppDebugCore.h"
#include "Utility/String/MyString.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG


#pragma comment(lib,"Dbghelp.lib")

WinAppDebugCore::WinAppDebugCore(const LPSTR& lpCmdLine) {
	SetUnhandledExceptionFilter(ExportDump);
	lpCmdLine;
#ifdef _DEBUG
	DebugLog("=====WinAppDebugCore=====");
#endif // _DEBUG

	// exeを起動したパス
	wchar_t fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	std::string exeName(ConvertString(fileName));
	
#ifdef _DEBUG
	// コマンド引数確認
	if (std::strcmp(lpCmdLine, "\0") != 0) {
		DebugLog("!!! EnebleCommandLineArguments !!!");
		DebugLog(std::format("EnebleCommand : {}", lpCmdLine));	
	} else {
		DebugLog("DisableCommandLineArguments");
	}
	DebugLog("");
#endif // _DEBUG
}

WinAppDebugCore::~WinAppDebugCore() {
}

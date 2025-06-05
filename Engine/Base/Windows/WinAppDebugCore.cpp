#include "WinAppDebugCore.h"
#include "../MyDebugLog.h"
#include "../MyString.h"

#pragma comment(lib,"Dbghelp.lib")

WinAppDebugCore::WinAppDebugCore(const LPSTR& lpCmdLine) {
	SetUnhandledExceptionFilter(ExportDump);

	MyDebugLog* myDebugLog = MyDebugLog::GetInstatnce();
	myDebugLog->Initialize();
	myDebugLog->Log("=====WinAppDebugCore=====");

	// exeを起動したパス
	wchar_t fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	std::string exeName(ConvertString(fileName));
	myDebugLog->Log("LaunchPath :" + exeName);
	
	// コマンド引数確認
	if (std::strcmp(lpCmdLine, "\0") != 0) {
		myDebugLog->Log("!!! EnebleCommandLineArguments !!!");
		myDebugLog->Log(std::format("EnebleCommand : {}\n", lpCmdLine));
	} else {
		myDebugLog->Log("DisableCommandLineArguments");
	}

	myDebugLog->Log("");
}

WinAppDebugCore::~WinAppDebugCore() {
	MyDebugLog* myDebugLog = MyDebugLog::GetInstatnce();
	myDebugLog->Finalize();
}

#pragma once
#define NOMINMAX
#include <Windows.h>
#include <dbghelp.h>
#include <strsafe.h>

namespace {
	LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
		SYSTEMTIME time;
		GetLocalTime(&time);
		wchar_t filePath[MAX_PATH] = { 0 };
		CreateDirectory(L"./Dumps", nullptr);
		StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
		HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
		// processId(こ�EexeのID)とクラチE��ュ(例夁Eの発生したthredIdを取征E
		DWORD processId = GetCurrentProcessId();
		DWORD threadId = GetCurrentThreadId();
		// 設定情報を�E劁E
		MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
		minidumpInfomation.ThreadId = threadId;
		minidumpInfomation.ExceptionPointers = exception;
		minidumpInfomation.ClientPointers = TRUE;
		// Dumpを�E劁E
		MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
		// 他に関連付けられてぁE��SEH例外ハンドラがあれ�E実行。通常はプロセスを終亁E��めE
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

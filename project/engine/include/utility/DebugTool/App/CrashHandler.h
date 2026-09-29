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
		// processId(このexeのID)とクラッシュ(例外の発生したthredIdを取得
		DWORD processId = GetCurrentProcessId();
		DWORD threadId = GetCurrentThreadId();
		// 設定情報を出力
		MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
		minidumpInfomation.ThreadId = threadId;
		minidumpInfomation.ExceptionPointers = exception;
		minidumpInfomation.ClientPointers = TRUE;
		// Dumpを出力
		MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
		// 関連するSEH例外ハンドラがあれば実行し、通常はプロセスを終了する
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

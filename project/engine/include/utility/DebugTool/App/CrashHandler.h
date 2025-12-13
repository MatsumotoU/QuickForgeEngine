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
		// processId(縺薙・exe縺ｮID)縺ｨ繧ｯ繝ｩ繝・す繝･(萓句､・縺ｮ逋ｺ逕溘＠縺殳hredId繧貞叙蠕・
		DWORD processId = GetCurrentProcessId();
		DWORD threadId = GetCurrentThreadId();
		// 險ｭ螳壽ュ蝣ｱ繧貞・蜉・
		MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
		minidumpInfomation.ThreadId = threadId;
		minidumpInfomation.ExceptionPointers = exception;
		minidumpInfomation.ClientPointers = TRUE;
		// Dump繧貞・蜉・
		MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
		// 莉悶↓髢｢騾｣莉倥￠繧峨ｌ縺ｦ縺・ｋSEH萓句､悶ワ繝ｳ繝峨Λ縺後≠繧後・螳溯｡後る壼ｸｸ縺ｯ繝励Ο繧ｻ繧ｹ繧堤ｵゆｺ・☆繧・
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

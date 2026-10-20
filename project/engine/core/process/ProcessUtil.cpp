#include "ProcessUtil.h"

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <mutex>
#include <vector>

namespace {
	class JobHandleRegistry {
	public:
		JobHandleRegistry() = default;
		JobHandleRegistry(const JobHandleRegistry&) = delete;
		JobHandleRegistry& operator=(const JobHandleRegistry&) = delete;
		JobHandleRegistry(JobHandleRegistry&&) = delete;
		JobHandleRegistry& operator=(JobHandleRegistry&&) = delete;

		~JobHandleRegistry() {
			for (HANDLE handle : handles_) {
				CloseHandle(handle);
			}
		}

		void Add(HANDLE handle) {
			std::scoped_lock lock(mutex_);
			handles_.push_back(handle);
		}

	private:
		std::mutex mutex_;
		std::vector<HANDLE> handles_;
	};

	JobHandleRegistry& GetJobHandleRegistry() {
		static JobHandleRegistry registry;
		return registry;
	}
}

bool QFE::ProcessUtil::LaunchExe(const std::string& exePath, const std::string& arguments) {
    std::string commandLine = exePath + " " + arguments;
    std::vector<char> buffer(commandLine.begin(), commandLine.end());
    buffer.push_back('\0');

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // 1. ジョブオブジェクトを作成する
    HANDLE hJob = CreateJobObjectA(nullptr, nullptr);
    if (hJob == nullptr) {
        return false;
    }

    // 2. 「親が死んだら子も道連れにする」という制限を設定する
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        CloseHandle(hJob);
        return false;
    }

    // ジョブへ登録する前に子プロセスが終了しないよう、一時停止状態で起動する。
    if (!CreateProcessA(nullptr, buffer.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        CloseHandle(hJob);
        return false;
    }

    if (!AssignProcessToJobObject(hJob, pi.hProcess)) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hJob);
        return false;
    }

    if (ResumeThread(pi.hThread) == static_cast<DWORD>(-1)) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hJob);
        return false;
    }

    // 親プロセスの終了までJob Objectを所有し、その後RAIIで解放する。
    GetJobHandleRegistry().Add(hJob);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

bool QFE::ProcessUtil::RunExeAndWait(
	const std::wstring& exePath,
	const std::wstring& arguments,
	const std::wstring& workingDirectory,
	const std::wstring& outputPath,
	uint32_t& exitCode) {
	exitCode = UINT32_MAX;

	SECURITY_ATTRIBUTES securityAttributes{};
	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.bInheritHandle = TRUE;
	const HANDLE outputHandle = CreateFileW(
		outputPath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		&securityAttributes,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (outputHandle == INVALID_HANDLE_VALUE) {
		return false;
	}

	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = outputHandle;
	startupInfo.hStdError = outputHandle;

	PROCESS_INFORMATION processInformation{};
	std::wstring commandLine = L"\"" + exePath + L"\" " + arguments;
	const BOOL created = CreateProcessW(
		exePath.c_str(),
		commandLine.data(),
		nullptr,
		nullptr,
		TRUE,
		CREATE_NO_WINDOW,
		nullptr,
		workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
		&startupInfo,
		&processInformation);
	CloseHandle(outputHandle);
	if (!created) {
		return false;
	}

	const DWORD waitResult = WaitForSingleObject(processInformation.hProcess, INFINITE);
	DWORD processExitCode = UINT32_MAX;
	const BOOL gotExitCode = GetExitCodeProcess(processInformation.hProcess, &processExitCode);
	CloseHandle(processInformation.hThread);
	CloseHandle(processInformation.hProcess);

	if (waitResult != WAIT_OBJECT_0 || !gotExitCode) {
		return false;
	}

	exitCode = processExitCode;
	return true;
}

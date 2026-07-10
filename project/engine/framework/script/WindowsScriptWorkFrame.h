#pragma once
#include <memory>
#include <string>

#define NOMINMAX
#include <Windows.h>

namespace QFE::SCRIPT {
	struct WindowsScriptInstance;
}

namespace QFE::FRAMEWORK {
	/// @brief DLLをロードします
	HMODULE LoadDll(const std::wstring& dllPath);
	/// @brief DLLをアンロードします
	bool UnloadDll(HMODULE dllHandle);

	/// @brief Windowsアプリケーション用のスクリプトインスタンスをロードする
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> LoadWindowsScriptInstance(
		const std::wstring& dllPath, const std::string& manifestFuncName);
	/// @brief Windowsアプリケーション用のスクリプトインスタンスをアンロードする
	void UnloadWindowsScriptInstance(QFE::SCRIPT::WindowsScriptInstance* scriptInstance);
}
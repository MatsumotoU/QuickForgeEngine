#pragma once
#include <memory>
#include <string>

namespace QFE::SCRIPT {
	struct WindowsScriptInstance;
}

namespace QFE::FRAMEWORK {
	/// @brief Windowsアプリケーション用のスクリプトインスタンスを作成する
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> CreateWindowsScriptInstance(const std::wstring& dllPath);
	/// @brief Windowsアプリケーション用のスクリプトインスタンスをアンロードする
	void UnloadWindowsScriptInstance(QFE::SCRIPT::WindowsScriptInstance* scriptInstance);
}
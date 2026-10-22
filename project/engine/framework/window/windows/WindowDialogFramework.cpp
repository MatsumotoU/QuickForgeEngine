#include "framework/window/WindowDialogFramework.h"

#include "window/GameWindowManager.h"
#include "EngineDefines.h"
using namespace QFE::FRAMEWORK;

#define NOMINMAX
#include <Windows.h>
#include <shlobj.h>
#include <string>

bool QFE::FRAMEWORK::RequestGetFilePathFromUser(
	const QFE::WINDOW::IGameWindowManager& windowManager, const std::wstring& filterName, const std::wstring& filterSpec,
	std::wstring& outFilePath,
	const std::wstring& initialDirectory) {
	// ウィンドウマネージャからメインウィンドウのハンドルを取得
	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_LOG("Error: The window manager is not a GameWindowManager.");
		return false;
	}
	const HWND mainWindow = gameWindowManager->GetMainWindow();
	if (mainWindow == nullptr) {
		QFE_LOG("Error: The window manager does not contain a main window.");
		return false;
	}
	// ファイルダイアログの設定
	OPENFILENAME ofn{};
	wchar_t filePathBuffer[MAX_PATH] = {};
	const std::wstring filter = filterName + L'\0' + filterSpec + L'\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainWindow;
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = filePathBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	if (!initialDirectory.empty()) {
		ofn.lpstrInitialDir = initialDirectory.c_str();
	}
	// ファイルダイアログを表示
	if (GetOpenFileName(&ofn)) {
		outFilePath = filePathBuffer;
		return true;
	} else {
		return false; // キャンセルまたはエラー
	}
}

bool QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(
	const QFE::WINDOW::IGameWindowManager& windowManager, const std::wstring& dialogTitle,
	std::wstring& outDirectoryPath) {
	// ウィンドウマネージャからメインウィンドウのハンドルを取得
	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_LOG("Error: The window manager is not a GameWindowManager.");
		return false;
	}
	const HWND mainWindow = gameWindowManager->GetMainWindow();
	if (mainWindow == nullptr) {
		QFE_LOG("Error: The window manager does not contain a main window.");
		return false;
	}
	// ディレクトリダイアログの設定
	BROWSEINFO bi{};
	wchar_t directoryPathBuffer[MAX_PATH] = {};
	bi.hwndOwner = mainWindow;
	bi.lpszTitle = dialogTitle.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != nullptr) {
		if (SHGetPathFromIDList(pidl, directoryPathBuffer)) {
			outDirectoryPath = directoryPathBuffer;
			CoTaskMemFree(pidl);
			return true;
		}
		CoTaskMemFree(pidl);
	}
	return false; // キャンセルまたはエラー
}

bool QFE::FRAMEWORK::RequestSaveFilePathFromUser(
	const QFE::WINDOW::IGameWindowManager& windowManager, const std::wstring& filterName, const std::wstring& filterSpec,
	std::wstring& outFilePath,
	const std::wstring& initialDirectory,
	const std::wstring& defaultExtension) {
	// ウィンドウマネージャからメインウィンドウのハンドルを取得
	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_LOG("Error: The window manager is not a GameWindowManager.");
		return false;
	}
	const HWND mainWindow = gameWindowManager->GetMainWindow();
	if (mainWindow == nullptr) {
		QFE_LOG("Error: The window manager does not contain a main window.");
		return false;
	}
	// 保存ダイアログの設定
	OPENFILENAME ofn{};
	wchar_t filePathBuffer[MAX_PATH] = {};
	const std::wstring filter = filterName + L'\0' + filterSpec + L'\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainWindow;
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = filePathBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	if (!initialDirectory.empty()) {
		ofn.lpstrInitialDir = initialDirectory.c_str();
	}
	if (!defaultExtension.empty()) {
		ofn.lpstrDefExt = defaultExtension.c_str();
	}
	// 保存ダイアログを表示
	if (GetSaveFileName(&ofn)) {
		outFilePath = filePathBuffer;
		return true;
	} else {
		return false; // キャンセルまたはエラー
	}
}

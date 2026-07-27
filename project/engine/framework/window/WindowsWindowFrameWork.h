#pragma once
#include <memory>
#include <string>
#include <cstdint>

#define NOMINMAX
#include <Windows.h>

namespace QFE {
	class GameWindowManager;
}

namespace QFE::FRAMEWORK {
	///  @brief WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::GameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height);

	/// @brief WindowManagerからウィンドウハンドルを取得する
	HWND GetWindowHandle(const GameWindowManager* windowManager, const std::string& windowName);

	/// @brief ユーザーにファイルパスを選択させるダイアログを表示し、選択されたファイルパスを取得する関数,スレッドをブロックするので注意してください
	/// @param hwnd ダイアログを表示する親ウィンドウのハンドル
	/// @param filterName ダイアログで表示するファイルタイプの名前（例: "Text Files"）ここはfilterSpecの説明文のようなもの
	/// @param filterSpec ダイアログで表示するファイルタイプの拡張子（例: "*.txt"）ここは絞り込みのための拡張子
	/// @param outFilePath 選択されたファイルパスを格納する変数
	/// @return ファイルが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
	bool RequestGetFilePathFromUser(
		HWND hwnd, const std::wstring& filterName, const std::wstring& filterSpec, std::wstring& outFilePath);

	/// @brief ユーザーにファイルを保存するダイアログを表示し、選択されたファイルパスを取得する関数,スレッドをブロックするので注意してください
	/// @param hwnd ダイアログを表示する親ウィンドウのハンドル
	/// @param filterName ダイアログで表示するファイルタイプの名前（例: "Text Files"）ここはfilterSpecの説明文のようなもの
	/// @param filterSpec ダイアログで表示するファイルタイプの拡張子（例: "*.txt"）ここは絞り込みのための拡張子
	/// @param outFilePath 選択されたファイルパスを格納する変数
	/// @return ファイルが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
	bool RequestSaveFilePathFromUser(
		HWND hwnd, const std::wstring& filterName, const std::wstring& filterSpec, std::wstring& outFilePath);

	/// @brief プロジェクトをコンパイルする関数。dotnetを使用して指定されたプロジェクトをビルドする。
	/// @param projectPath コンパイルするプロジェクトファイルのパス（例: "C:\\path\\to\\project.vcxproj"）
	/// @param outputDir ビルド成果物を出力するディレクトリのパス（例: "C:\\path\\to\\output"）何も入れない場合同じ場所に出力されます
	/// @param isRelease リリースビルドかデバッグビルドかを指定するフラグ。trueの場合はリリースビルド、falseの場合はデバッグビルド。
	bool CompileProject(const std::wstring& projectPath, const std::wstring& outputDir, bool isRelease);

	/// @brief メインウィンドウがアクティブかどうかを判定する関数
	bool IsMainWindowActive(const GameWindowManager* windowManager);
}

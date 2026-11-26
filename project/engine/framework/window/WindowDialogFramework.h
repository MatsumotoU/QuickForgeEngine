#pragma once
#include <string>

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::FRAMEWORK {
	/// @brief 親ウィンドウの生成前にも表示できるエラーダイアログ。
	/// @details 閉じられるまで待機する。タイトルと本文は呼び出し元の文字列をそのまま使う。
	void ShowWindowError(const std::wstring& message, const std::wstring& title);
	/// @brief 親ウィンドウがない起動時に、指定された種類のファイルを選択する。
	/// @return 選択時は true。キャンセルまたはエラー時は false で出力を変更しない。
	bool RequestStartupFilePath(const std::wstring& filterName, const std::wstring& filterSpec,
		std::wstring& outFilePath);

	/// @brief ユーザーにファイルパスを選択させるダイアログを表示し、選択されたファイルパスを取得する関数,スレッドをブロックするので注意してください
	/// @param windowManager ダイアログを表示する親ウィンドウを管理するウィンドウマネージャ
	/// @param filterName ダイアログで表示するファイルタイプの名前（例: "Text Files"）ここはfilterSpecの説明文のようなもの
	/// @param filterSpec ダイアログで表示するファイルタイプの拡張子（例: "*.txt"）ここは絞り込みのための拡張子
	/// @param outFilePath 選択されたファイルパスを格納する変数
	/// @return ファイルが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
	bool RequestGetFilePathFromUser(
		const QFE::WINDOW::IGameWindowManager& windowManager, const std::wstring& filterName, const std::wstring& filterSpec,
		std::wstring& outFilePath,
		const std::wstring& initialDirectory = std::wstring());

	/// @brief ユーザーにディレクトリを選択させるダイアログを表示し、選択されたディレクトリパスを取得する関数,スレッドをブロックするので注意してください
	/// @param windowManager ダイアログを表示する親ウィンドウを管理するウィンドウマネージャ
	/// @param dialogTitle ダイアログに表示するタイトル（例: "Select Directory"）
	/// @param outDirectoryPath 選択されたディレクトリパスを格納する変数
	/// @return ディレクトリが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
	bool RequestGetDirectoryPathFromUser(
		const QFE::WINDOW::IGameWindowManager& windowManager,
		const std::wstring& dialogTitle,
		std::wstring& outDirectoryPath);

	/// @brief ユーザーにファイルを保存するダイアログを表示し、選択されたファイルパスを取得する関数,スレッドをブロックするので注意してください
	/// @param windowManager ダイアログを表示する親ウィンドウを管理するウィンドウマネージャ
	/// @param filterName ダイアログで表示するファイルタイプの名前（例: "Text Files"）ここはfilterSpecの説明文のようなもの
	/// @param filterSpec ダイアログで表示するファイルタイプの拡張子（例: "*.txt"）ここは絞り込みのための拡張子
	/// @param outFilePath 選択されたファイルパスを格納する変数
	/// @return ファイルが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
	bool RequestSaveFilePathFromUser(
		const QFE::WINDOW::IGameWindowManager& windowManager, const std::wstring& filterName, const std::wstring& filterSpec,
		std::wstring& outFilePath,
		const std::wstring& initialDirectory = std::wstring(),
		const std::wstring& defaultExtension = std::wstring());
}

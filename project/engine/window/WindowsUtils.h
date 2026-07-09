#pragma once
#define NOMINMAX
#include <windows.h>
#include <string>

namespace QFE::WINDOW {
	/// @brief ユーザーにファイルパスを選択させるダイアログを表示し、選択されたファイルパスを取得する関数,スレッドをブロックするので注意してください
	/// @param hwnd ダイアログを表示する親ウィンドウのハンドル
	/// @param filterName ダイアログで表示するファイルタイプの名前（例: "Text Files"）ここはfilterSpecの説明文のようなもの
	/// @param filterSpec ダイアログで表示するファイルタイプの拡張子（例: "*.txt"）ここは絞り込みのための拡張子
	/// @param outFilePath 選択されたファイルパスを格納する変数
	/// @return ファイルが選択され、パスが取得できた場合はtrue、キャンセルされた場合やエラーが発生した場合はfalse
    bool RequestGetFilePathFromUser(
        HWND hwnd, const std::wstring& filterName, const std::wstring& filterSpec, std::wstring& outFilePath);
}

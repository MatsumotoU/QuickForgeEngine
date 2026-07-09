#include "WindowsUtils.h"
#include <shobjidl.h>
#include <thread>

bool QFE::WINDOW::RequestGetFilePathFromUser(
    HWND hwnd, const std::wstring& filterName, const std::wstring& filterSpec, std::wstring& outFilePath) {

    OPENFILENAME ofn;            // 共通ダイアログボックスの構造体
    wchar_t szFile[MAX_PATH] = { 0 }; // 選択されたファイル名を受け取るバッファ

    // 1. 構造体の初期化
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;        // 親ウィンドウを安全に指定できます
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);

    // 2. 引数のフィルターを設定
    std::wstring filterStr = filterName + L'\0' + filterSpec + L"\0All Files\0*.*\0";
    ofn.lpstrFilter = filterStr.c_str();
    ofn.nFilterIndex = 1;

    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;

    // OFN_FILEMUSTEXIST: 存在するファイルしか選べないようにする
    // OFN_PATHMUSTEXIST: 存在するフォルダしか選べないようにする
    // OFN_NOCHANGEDIR:   作業ディレクトリを変更しないようにする
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // 3. ダイアログを表示
    if (GetOpenFileName(&ofn) == TRUE) {
        outFilePath = ofn.lpstrFile; // 取得成功
        return true;
    }

    return false; // キャンセル、またはエラー
}

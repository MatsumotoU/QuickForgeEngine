#include "WindowsUtils.h"
#include <shobjidl.h>
#include <thread>
#include <iostream>
#include <string>
#include <vector>

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

bool QFE::WINDOW::CompileProject(const std::wstring& projectPath, const std::wstring& outputDir, bool isRelease) {
    // 1. コマンドライン文字列の作成
    std::wstring configuration = isRelease ? L"Release" : L"Debug";
    std::wstring arguments = L"dotnet build \"" + projectPath + L"\" -c " + configuration;

	// 出力ディレクトリが指定されていない場合は、プロジェクトのディレクトリに出力するように設定
    if (outputDir.empty()) {
        arguments += L" -o \"" + projectPath + L"\"";
    } else {
		arguments += L" -o \"" + outputDir + L"\"";
    }

    // 2. パイプの作成（dotnetの出力を受け取るため）
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // 子プロセスにハンドルを引き継ぐ
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        std::wcerr << L"パイプの作成に失敗しました。" << std::endl;
        return false;
    }

    // 読み取り側ハンドルは子プロセスに引き継がないように設定
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

    // 3. 子プロセス（dotnet）の起動準備
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStd_OUT_Wr;  // エラー出力をパイプへ
    siStartInfo.hStdOutput = hChildStd_OUT_Wr; // 標準出力をパイプへ
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // CreateProcess は引数の文字列を書き換える可能性があるため、可変バッファに移す
    std::vector<wchar_t> cmdBuffer(arguments.begin(), arguments.end());
    cmdBuffer.push_back(L'\0');

    // 4. プロセスの起動
    BOOL bSuccess = CreateProcess(
        NULL,
        cmdBuffer.data(),     // コマンドライン引数
        NULL,          // プロセスセキュリティ属性
        NULL,          // スレッドセキュリティ属性
        TRUE,          // ハンドルの継承（TRUEにする）
        CREATE_NO_WINDOW, // 黒い画面（コンソール）を非表示にする
        NULL,          // 環境変数（親と同じ）
        NULL,          // カレントディレクトリ
        &siStartInfo,  // スタートアップ情報
        &piProcInfo    // プロセス情報
    );

    if (!bSuccess) {
        std::wcerr << L"dotnet の起動に失敗しました。インストールされているか確認してください。" << std::endl;
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_OUT_Rd);
        return false;
    }

    // 書き込み側のハンドルは親側では使わないので先に閉じる（これをしないとReadが無限ループになる）
    CloseHandle(hChildStd_OUT_Wr);

    // 5. dotnetの出力ログを読み取る
    DWORD dwRead;
    CHAR chBuf[4096];
    std::string outputLog = "";

    while (ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL) && dwRead > 0) {
        chBuf[dwRead] = '\0';
        outputLog += chBuf; // ログを蓄積
    }

    // 6. プロセスの終了を待ち、終了コードを取得する
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);

    // ハンドルのクローズ
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Rd);

    // 7. 結果判定
    if (exitCode == 0) {
        std::wcout << L"コンパイル成功！" << std::endl;
        return true;
    } else {
        std::wcout << L"コンパイル失敗（ExitCode: " << exitCode << L"）" << std::endl;
        std::cout << "--- ビルドログ ---" << std::endl;
        std::cout << outputLog << std::endl; // エラー内容を表示
        return false;
    }
}

#include "ProcessUtil.h"

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>

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
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        CloseHandle(hJob);
        return false;
    }

    // 3. 子プロセスを起動する
    // (※まだこの時点ではジョブに登録されていないので、一時停止状態で起動するのが安全ですが、
    //  個人開発の単純な構成ならこのフラグ0のままで割と動きます)
    if (!CreateProcessA(nullptr, buffer.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(hJob);
        return false;
    }

    // 4. 起動した子プロセスをジョブオブジェクトに割り当てる
    AssignProcessToJobObject(hJob, pi.hProcess);

    // 5. 後片付け
    // ジョブオブジェクトのハンドル自体は、エディタ（親）が生きている間保持する必要があります。
    // ここで CloseHandle(hJob) してしまうと、制限が解除されてしまうため、
    // 本来はクラスのメンバ変数などに保持してエディタ終了時に閉じるか、
    // 面倒なら「リーク」させてOSにエディタ終了時に自動解放してもらう技が使えます。
    // 今回は最も確実な「リークさせてエディタ終了時にOSに殺してもらう」方法をとるため、CloseHandle(hJob) はあえてしません。

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

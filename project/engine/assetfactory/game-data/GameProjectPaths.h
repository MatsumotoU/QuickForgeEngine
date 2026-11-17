#pragma once
#include <filesystem>
#include <string>
#ifdef _WIN32
#include <cwchar>
#endif

namespace QFE::PROJECT {
/// @brief プロセス内で選択されたプロジェクトのルートを保持する。
inline std::filesystem::path& RootStorage() { static std::filesystem::path root; return root; }
/// @brief 実行中のプロジェクトまたは展開済みパッケージのルートを設定する。
inline void SetRoot(const std::filesystem::path& root) { RootStorage() = root; }
/// @brief 実行中のプロジェクトのルートを取得する。未設定時は空。
inline std::filesystem::path Root() { return RootStorage(); }
/// @brief アセットのルートを取得する。プロジェクト未設定時は resources/ を返す。
inline std::filesystem::path Assets() {
    const auto root = Root();
    return root.empty() ? std::filesystem::path("resources") : root / "assets";
}
/// @brief シンボリックリンクを解決し、パスが指定ディレクトリの内側か判定する。
/// @param path 判定するファイルまたはディレクトリのパス。
/// @param directory 基準となるディレクトリ。
/// @return 基準ディレクトリの内側であれば true。基準自身は含まない。
inline bool IsWithin(const std::filesystem::path& path, const std::filesystem::path& directory) {
    std::error_code error;
    const auto base = std::filesystem::weakly_canonical(directory, error);
    if (error) return false;
    const auto target = std::filesystem::weakly_canonical(path, error);
    if (error) return false;
    auto basePart = base.begin();
    auto targetPart = target.begin();
    for (; basePart != base.end(); ++basePart, ++targetPart) {
        if (targetPart == target.end()) return false;
#ifdef _WIN32
        if (_wcsicmp(basePart->c_str(), targetPart->c_str()) != 0) return false;
#else
        if (*basePart != *targetPart) return false;
#endif
    }
    return targetPart != target.end();
}
/// @brief アセットのパスを現在のプロジェクトのルートから解決する。
inline std::filesystem::path Resolve(const std::filesystem::path& input) {
    if (Root().empty() || input.is_absolute()) return input;
    auto it = input.begin();
    if (it != input.end() && *it == "resources") {
        std::filesystem::path rest;
        ++it;
        if (it != input.end() && *it == "scene") { rest /= "scenes"; ++it; }
        for (; it != input.end(); ++it) rest /= *it;
        return (Assets() / rest).lexically_normal();
    }
    return (Assets() / input).lexically_normal();
}
}

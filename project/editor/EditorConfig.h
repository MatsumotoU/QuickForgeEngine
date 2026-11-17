#pragma once
#include <filesystem>
#include <string>

namespace QFE::EDITOR {
    struct EditorConfig {
        // 相対パスは起動時の作業ディレクトリを基準にする。
        std::string projectRoot;
        bool Load(const std::filesystem::path& path);
    };
}

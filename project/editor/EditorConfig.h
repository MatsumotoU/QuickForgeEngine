#pragma once
#include <filesystem>
#include <string>

namespace QFE::EDITOR {
    /// @brief エディタ全体で使用するプロジェクト選択設定。
    struct EditorConfig {
        /// @brief ゲームプロジェクトの格納先。相対パスは project/ を基準とする。
        std::string gameProjectDirectory = "../game-projects";
        /// @brief 最後に選択した編集用プロジェクトのパス。
        std::string lastProject;
        /// @brief 設定ファイルを読み込み、有効な値を各項目に反映する。
        /// @param path 設定ファイルのパス。
        /// @return 読み込みに成功した場合は true。
        bool Load(const std::filesystem::path& path);
        /// @brief 既存の設定項目を保持しながらエディタ設定を書き込む。
        /// @param path 設定ファイルのパス。
        /// @return 保存に成功した場合は true。
        bool Save(const std::filesystem::path& path) const;
    };
}

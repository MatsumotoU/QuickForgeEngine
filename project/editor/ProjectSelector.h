#pragma once
#include "EditorConfig.h"
#include <filesystem>
#include <string>
#include <vector>

namespace QFE::WINDOW { class IGameWindowManager; }
namespace QFE::EDITOR {
    /// @brief ゲームプロジェクトの選択と作成を行う ImGui の起動画面。
    /// @details エンジンの初期化と終了は呼び出し元が行う。
    class ProjectSelector final {
    public:
        /// @brief エディタ設定を使ってプロジェクトの一覧を準備する。
        /// @param config プロジェクトの格納先を保持する設定。
        /// @param configPath 設定の変更を保存するファイルのパス。
        ProjectSelector(EditorConfig& config, std::filesystem::path configPath);
        ProjectSelector(const ProjectSelector&) = delete;
        ProjectSelector& operator=(const ProjectSelector&) = delete;
        ProjectSelector(ProjectSelector&&) = delete;
        ProjectSelector& operator=(ProjectSelector&&) = delete;

        /// @brief 選択画面を描画し、選択・新規作成・格納先の変更を処理する。
        /// @param windowManager ディレクトリ選択ダイアログの親ウィンドウ。
        void Draw(QFE::WINDOW::IGameWindowManager& windowManager);
        /// @brief 選択されたプロジェクトのルートを取得する。未選択時は空。
        const std::filesystem::path& SelectedProject() const { return selectedProject_; }
        /// @brief 選択画面がキャンセルされたかを取得する。
        bool Cancelled() const { return cancelled_; }
    private:
        /// @brief 設定された格納先からプロジェクトの一覧を再取得する。
        void RefreshProjects();
        /// @brief 既存プロジェクトを選択し、必要なディレクトリを補う。
        void OpenProject(const std::filesystem::path& root);
        /// @brief 入力された名前でプロジェクトを作成して選択する。
        void CreateProject();
        /// @brief 入力された格納先を設定に保存して一覧を更新する。
        void ChangeDirectory();
        EditorConfig& config_;
        std::filesystem::path configPath_;
        std::filesystem::path base_;
        std::filesystem::path selectedProject_;
        std::vector<std::filesystem::path> projects_;
        std::string directoryInput_;
        std::string projectName_;
        std::string status_;
        int selectedIndex_ = -1;
        bool cancelled_ = false;
    };
}

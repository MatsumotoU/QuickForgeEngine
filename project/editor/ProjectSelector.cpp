#include "ProjectSelector.h"
#include "framework/window/WindowDialogFramework.h"
#include "string/MyString.h"
#include "scene/SceneManager.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

QFE::EDITOR::ProjectSelector::ProjectSelector(EditorConfig& config, std::filesystem::path configPath, SCENE::SceneManager& sceneManager)
    : config_(config), sceneManager_(sceneManager), configPath_(std::move(configPath)), directoryInput_(config.gameProjectDirectory) {
    RefreshProjects();
}

void QFE::EDITOR::ProjectSelector::RefreshProjects() {
    // editor-config.json の場所から project.json を持つディレクトリだけを列挙する。
    projects_.clear();
    selectedIndex_ = -1;
    std::error_code error;
    base_ = std::filesystem::absolute(std::filesystem::path(QFE::ConvertString(config_.gameProjectDirectory)), error);
    if (!error) std::filesystem::create_directories(base_, error);
    if (error) { status_ = "Could not open the game project directory."; return; }
    for (std::filesystem::directory_iterator it(base_, error), end; !error && it != end; it.increment(error)) {
        std::error_code entryError;
        if (it->is_directory(entryError) && std::filesystem::is_regular_file(it->path() / L"project.json", entryError)) {
            projects_.push_back(it->path());
        }
    }
    if (error) { status_ = "Could not list game projects."; return; }
    std::sort(projects_.begin(), projects_.end());
}

void QFE::EDITOR::ProjectSelector::OpenProject(const std::filesystem::path& root) {
    // 古いプロジェクトには entities がない場合があるため、開くときに補う。
    std::error_code error;
    std::filesystem::create_directories(root / L"assets" / L"entities", error);
    if (error) { status_ = "Could not create the project's entities directory."; return; }
    selectedProject_ = root;
}

void QFE::EDITOR::ProjectSelector::CreateProject() {
    // フォルダ名を単一の安全な要素に限定し、他の場所への作成を防ぐ。
    const bool valid = !projectName_.empty() && projectName_.size() <= 80 &&
        std::all_of(projectName_.begin(), projectName_.end(), [](unsigned char c) {
            return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
        });
    if (!valid) { status_ = "Use lowercase letters, numbers, hyphens or underscores (up to 80 characters)."; return; }
    const auto root = base_ / projectName_;
    std::error_code error;
    if (!std::filesystem::create_directory(root, error)) { status_ = "The project already exists or could not be created."; return; }
    for (const auto* dir : { L"scenes", L"entities", L"models", L"images", L"audio", L"scripts", L"shaders", L"animations" }) {
        std::filesystem::create_directories(root / L"assets" / dir, error);
        if (error) break;
    }
    if (error) { status_ = "Could not finish creating the project. Check the project directory."; return; }
    // 初期カメラとライトを含むシーンを通常のシリアライズ処理で保存する。
    sceneManager_.Initialize();
    std::ofstream scene(root / L"assets" / L"scenes" / L"default.scene", std::ios::binary);
    scene << sceneManager_.CreateSnapshot().sceneJson.dump(4) << '\n';
    scene.close();
    if (!scene) { status_ = "Could not create the default scene."; return; }
    std::ofstream file(root / L"project.json", std::ios::binary);
    file << nlohmann::json({ {"name", projectName_}, {"version", 1}, {"start_scene", "default.scene"} }).dump(4) << '\n';
    file.close();
    if (!file) { status_ = "Could not finish creating the project. Check the project directory."; return; }
    createdProject_ = true;
    selectedProject_ = root;
}

void QFE::EDITOR::ProjectSelector::ChangeDirectory() {
    // 保存が成功した場合だけ選択画面の参照先も切り替える。
    if (directoryInput_.empty()) { status_ = "Enter a project directory."; return; }
    std::error_code error;
    const auto path = std::filesystem::absolute(std::filesystem::path(QFE::ConvertString(directoryInput_)), error);
    if (!error) std::filesystem::create_directories(path, error);
    if (error || !std::filesystem::is_directory(path, error)) { status_ = "Could not open the game project directory."; return; }
    EditorConfig updated = config_;
    updated.gameProjectDirectory = directoryInput_;
    if (!updated.Save(configPath_)) { status_ = "Could not save editor-config.json."; return; }
    config_ = std::move(updated);
    status_.clear();
    RefreshProjects();
}

void QFE::EDITOR::ProjectSelector::Draw(QFE::WINDOW::IGameWindowManager& windowManager) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(620, 470), ImGuiCond_FirstUseEver);
    ImGui::Begin("Select Game Project", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::TextUnformatted("Game projects directory");
    ImGui::SetNextItemWidth(-175.0f);
    ImGui::InputText("##directory", &directoryInput_);
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        std::wstring path;
        if (QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(windowManager, L"Select game projects directory", path)) {
            directoryInput_ = QFE::ConvertString(path);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply")) ChangeDirectory();
    ImGui::Separator();
    ImGui::TextUnformatted("Projects");
    if (ImGui::BeginListBox("##projects", ImVec2(-1, 180))) {
        for (size_t i = 0; i < projects_.size(); ++i) {
            const std::string label = QFE::ConvertString(projects_[i].filename().wstring());
            if (ImGui::Selectable(label.c_str(), selectedIndex_ == static_cast<int>(i))) selectedIndex_ = static_cast<int>(i);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) OpenProject(projects_[i]);
        }
        ImGui::EndListBox();
    }
    if (ImGui::Button("Open selected") && selectedIndex_ >= 0 && static_cast<size_t>(selectedIndex_) < projects_.size()) OpenProject(projects_[static_cast<size_t>(selectedIndex_)]);
    ImGui::Separator();
    ImGui::TextUnformatted("New project name (lowercase)");
    ImGui::SetNextItemWidth(-160.0f);
    ImGui::InputText("##name", &projectName_);
    ImGui::SameLine();
    if (ImGui::Button("Create and open")) CreateProject();
    if (!status_.empty()) ImGui::TextWrapped("%s", status_.c_str());
    if (ImGui::Button("Cancel")) cancelled_ = true;
    ImGui::End();
}

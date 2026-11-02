#include "EditorConfig.h"
#include <fstream>
#include <nlohmann/json.hpp>

bool QFE::EDITOR::EditorConfig::Load(const std::filesystem::path& path) {
    // 壊れた設定を無理に読み込まず、呼び出し元の既定値を残す。
    std::ifstream file(path);
    if (!file) return false;
    try {
        const auto json = nlohmann::json::parse(file);
        const auto it = json.find("game_project_directory");
        if (!json.is_object() || it == json.end() || !it->is_string() || it->get<std::string>().empty()) return false;
        gameProjectDirectory = it->get<std::string>();
        const auto last = json.find("last_project");
        if (last != json.end() && last->is_string()) lastProject = last->get<std::string>();
        return true;
    } catch (const nlohmann::json::exception&) { return false; }
}

bool QFE::EDITOR::EditorConfig::Save(const std::filesystem::path& path) const {
    // 別の設定項目を消さないよう、既存の JSON を読み込んでから更新する。
    nlohmann::json json = nlohmann::json::object();
    std::ifstream input(path);
    if (input) {
        try { json = nlohmann::json::parse(input); }
        catch (const nlohmann::json::exception&) { return false; }
        if (!json.is_object()) return false;
    }
    input.close();
    json["game_project_directory"] = gameProjectDirectory;
    json["last_project"] = lastProject;
    std::ofstream output(path, std::ios::trunc);
    if (!output) return false;
    output << json.dump(4) << '\n';
    return output.good();
}

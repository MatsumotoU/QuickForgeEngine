#include "EditorConfig.h"
#include <fstream>
#include <nlohmann/json.hpp>

bool QFE::EDITOR::EditorConfig::Load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) return false;
    try {
        const auto json = nlohmann::json::parse(file);
        if (!json.is_object()) return false;
        const auto it = json.find("project_root");
        if (it == json.end() || !it->is_string() || it->get<std::string>().empty()) return false;
        projectRoot = it->get<std::string>();
        return true;
    } catch (const nlohmann::json::exception&) { return false; }
}

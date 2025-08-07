#pragma once
#include <string>
#include <fstream>
#include <filesystem>

class LuaScriptGenerator {
public:
    // Luaスクリプトのデフォルト内容を返す
    static std::string GetDefaultScriptContent() {
        return
            "function Init()\n"
            "\tDebugLog(\"Init\")\n"
            "end\n\n"
            "function Update()\n"
            "\tDebugLog(\"Update\")\n"
            "end\n\n"
            "function Collision()\n"
            "\tDebugLog(\"IsHit\")\n"
            "end";
    }

    // ファイル名の重複を避けて新しいファイル名を生成
    static std::string GenerateUniqueFileName(const std::string& directory, const std::string& baseName) {
        std::string fileName = baseName;
        std::string fullPath = directory + "/" + fileName;
        int count = 1;
        while (std::filesystem::exists(fullPath)) {
            fileName = baseName.substr(0, baseName.find_last_of('.')) + "_" + std::to_string(count) + ".lua";
            fullPath = directory + "/" + fileName;
            ++count;
        }
        return fileName;
    }

    // Luaスクリプトファイルを生成（成功: true, 失敗: false）
    static bool CreateLuaScriptFile(const std::string& directory, const std::string& fileName, const std::string& content, std::string& outFullPath) {
        // ディレクトリがなければ作成
        std::filesystem::create_directories(directory);

        outFullPath = directory + "/" + fileName;
        std::ofstream ofs(outFullPath);
        if (!ofs) {
            return false;
        }
        ofs << content;
        ofs.close();
        return true;
    }
};
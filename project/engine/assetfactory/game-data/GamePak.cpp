#include "GamePak.h"
#include "GameProjectPaths.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace QFE::PAK {
// qfepak v1/v2: 8バイトの識別子、LE32バージョン、LE32件数、
// 続いて「LE32パス長、LE64データ長、UTF-8相対パス、データ本体」を件数分並べる。
// 中身は無圧縮。C++構造体のメモリ配置やCPUのバイト順には依存しない。

void WriteNumber(std::ostream& out, std::uint64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) out.put(static_cast<char>((value >> (i * 8)) & 0xff));
}
bool ReadNumber(std::istream& in, unsigned bytes, std::uint64_t& value) {
    value = 0;
    for (unsigned i = 0; i < bytes; ++i) {
        const int byte = in.get();
        if (byte == std::char_traits<char>::eof()) return false;
        value |= static_cast<std::uint64_t>(byte) << (i * 8);
    }
    return true;
}

// アーカイブ内のパスはプロジェクト設定、登録情報、assets/、sources/ に限定する。
// 絶対パス、親への移動、Windowsドライブ名や区切り文字の混入を拒否する。
bool ValidName(const std::string& name) {
    if (name.empty() || name.size() > MaxPathBytes || name.find_first_of("\\:\0", 0, 3) != std::string::npos ||
        name.find_first_of("<>|?*") != std::string::npos) return false;
    if (name != "project.json" && name != "asset-registry.json" &&
        !name.starts_with("assets/") && !name.starts_with("sources/")) return false;
    size_t begin = 0;
    while (begin < name.size()) {
        const size_t end = name.find('/', begin);
        const auto part = name.substr(begin, end == std::string::npos ? end : end - begin);
        if (part.empty() || part == "." || part == ".." || part.back() == '.' || part.back() == ' ') return false;
        for (const char c : part) if (static_cast<unsigned char>(c) < 32) return false;
        // Windows のデバイス名は拡張子付きでも通常ファイルではない。
        std::string stem = part.substr(0, part.find('.'));
        for (char& c : stem) {
            if (c >= 'a' && c <= 'z') c = static_cast<char>(c - ('a' - 'A'));
        }
        if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL" ||
            (stem.size() == 4 && (stem.starts_with("COM") || stem.starts_with("LPT")) && stem[3] >= '1' && stem[3] <= '9')) return false;
        if (end == std::string::npos) return true;
        begin = end + 1;
    }
    return false;
}
std::filesystem::path Utf8Path(const std::string& text) {
    return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
}
std::string Utf8Name(const std::filesystem::path& path) {
    const auto text = path.generic_u8string();
    return std::string(reinterpret_cast<const char*>(text.data()), text.size());
}
bool CopyBytes(std::istream& in, std::ostream& out, std::uint64_t size) {
    std::array<char, 65536> buffer{};
    while (size) {
        const auto chunk = static_cast<std::streamsize>((std::min)(size, static_cast<std::uint64_t>(buffer.size())));
        if (!in.read(buffer.data(), chunk)) return false;
        if (!out.write(buffer.data(), chunk)) return false;
        size -= static_cast<std::uint64_t>(chunk);
    }
    return true;
}

// 完成した一時ファイルを最後に移動し、失敗時に既存の出力ファイルを壊さない。
bool ReplaceFile(const std::filesystem::path& source, const std::filesystem::path& target, std::error_code& error) {
#ifdef _WIN32
    if (MoveFileExW(source.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING) != 0) { error.clear(); return true; }
    error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
    return false;
#else
    std::filesystem::rename(source, target, error);
    return !error;
#endif
}

bool Write(const std::filesystem::path& project, const std::filesystem::path& output, std::string& errorText,
                  const std::string& previewScene) {
    std::filesystem::path temporary;
    bool ownsTemporary = false;
    try {
        // 拡張子の入力間違いによって project.json 等を上書きしないようにする。
        auto extension = output.extension().wstring();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](wchar_t c) {
            return c >= L'A' && c <= L'Z' ? static_cast<wchar_t>(c + (L'a' - L'A')) : c;
        });
        if (extension != L".qfepak") { errorText = "The output must have a .qfepak extension."; return false; }
        std::ifstream meta(project / "project.json", std::ios::binary);
        const auto json = nlohmann::json::parse(meta);
        const auto start = previewScene.empty() ? json.value("start_scene", std::string{}) : previewScene;
        if (start.empty()) { errorText = "Set a start scene before exporting."; return false; }
        auto scene = Utf8Path(start);
        const auto scenes = project / "assets" / "scenes";
		if (scene.extension().empty()) {
			auto modern = scene;
			modern += ".scene";
			auto legacy = scene;
			legacy += ".json";
			scene = std::filesystem::is_regular_file(scenes / modern) ? modern : legacy;
		}
        if (!QFE::PROJECT::IsWithin(scenes / scene, scenes) || !std::filesystem::is_regular_file(scenes / scene)) {
            errorText = "The start scene was not found inside assets/scenes."; return false;
        }
        // 出力先が assets 内だと、以前の pak を取り込んで自己増殖するため禁止する。
        if (QFE::PROJECT::IsWithin(output, project / "assets") || QFE::PROJECT::IsWithin(output, project / "sources")) {
            errorText = "Choose an output outside assets and sources."; return false;
        }
        std::vector<std::filesystem::path> files{ project / "project.json" };
        std::error_code error;
        const auto registry = project / "asset-registry.json";
        if (std::filesystem::is_symlink(registry, error)) { errorText = "Symbolic links cannot be exported."; return false; }
        if (error && error != std::errc::no_such_file_or_directory) { errorText = "Could not inspect asset registry."; return false; }
        error.clear();
        if (std::filesystem::is_regular_file(registry, error)) files.push_back(registry);
        if (error && error != std::errc::no_such_file_or_directory) { errorText = "Could not inspect asset registry."; return false; }
        error.clear();
        for (const auto& directory : { project / "assets", project / "sources" }) {
            if (std::filesystem::is_symlink(directory, error)) { errorText = "Symbolic links cannot be exported."; return false; }
            if (error && error != std::errc::no_such_file_or_directory) { errorText = "Could not inspect project directory."; return false; }
            error.clear();
            if (!std::filesystem::exists(directory, error)) {
                if (error || directory.filename() == "assets") { errorText = "Could not find project assets."; return false; }
                continue;
            }
            for (std::filesystem::recursive_directory_iterator it(directory, error), end; !error && it != end; it.increment(error)) {
                if (it->is_symlink(error)) { errorText = "Symbolic links cannot be exported."; return false; }
                if (it->is_regular_file(error)) files.push_back(it->path());
            }
            if (error) break;
        }
        if (error || files.size() > MaxEntries) { errorText = "Could not enumerate project files."; return false; }
        std::sort(files.begin() + 1, files.end());
        temporary = output;
        temporary += L".qfe-writing";
        if (std::filesystem::exists(temporary)) { errorText = "A temporary export file already exists."; return false; }
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) { errorText = "Could not create the package."; return false; }
        ownsTemporary = true;
        out.write(Magic.data(), static_cast<std::streamsize>(Magic.size()));
        WriteNumber(out, Version, 4); WriteNumber(out, files.size(), 4);
        bool success = true;
        for (const auto& file : files) {
            const auto name = Utf8Name(file.lexically_relative(project));
            const auto size = std::filesystem::file_size(file, error);
            std::ifstream in(file, std::ios::binary);
            if (error || !in || !ValidName(name)) { success = false; break; }
            WriteNumber(out, name.size(), 4); WriteNumber(out, size, 8);
            out.write(name.data(), static_cast<std::streamsize>(name.size()));
            if (!out || !CopyBytes(in, out, size)) { success = false; break; }
        }
        out.close();
        if (!success || !out) {
            std::filesystem::remove(temporary, error);
            errorText = "Failed to write the package contents."; return false;
        }
        std::error_code replaceError;
        if (!ReplaceFile(temporary, output, replaceError)) {
            std::filesystem::remove(temporary, error);
            errorText = "Failed to replace the package (" + std::to_string(replaceError.value()) + "): " + replaceError.message(); return false;
        }
        return true;
    } catch (const std::exception&) {
        if (ownsTemporary) { std::error_code error; std::filesystem::remove(temporary, error); }
        errorText = "Could not read the project or create the package."; return false;
    }
}

// 読み込み途中でも必ず全サイズを検証し、パッケージ外への書き込みを防ぐ。
bool Extract(const std::filesystem::path& pak, const std::filesystem::path& directory, std::string& errorText) {
    try {
        std::ifstream in(pak, std::ios::binary);
        std::array<char, 8> magic{};
        std::uint64_t version = 0, count = 0;
        if (!in.read(magic.data(), static_cast<std::streamsize>(magic.size())) || magic != Magic ||
            !ReadNumber(in, 4, version) || (version != 1 && version != Version) || !ReadNumber(in, 4, count) || count == 0 || count > MaxEntries) {
            errorText = "Invalid or unsupported qfepak header."; return false;
        }
        std::error_code error;
        const auto fileSize = std::filesystem::file_size(pak, error);
        if (error) { errorText = "Could not read the package size."; return false; }
        for (std::uint64_t i = 0; i < count; ++i) {
            std::uint64_t pathSize = 0, size = 0;
            if (!ReadNumber(in, 4, pathSize) || !ReadNumber(in, 8, size) || pathSize == 0 || pathSize > MaxPathBytes) {
                errorText = "Invalid package entry."; return false;
            }
            std::string name(static_cast<size_t>(pathSize), '\0');
            if (!in.read(name.data(), static_cast<std::streamsize>(name.size())) || !ValidName(name) ||
                (version == 1 && name != "project.json" && !name.starts_with("assets/"))) {
                errorText = "Invalid package path."; return false;
            }
            const auto pos = in.tellg();
            if (pos < 0 || static_cast<std::uint64_t>(pos) > fileSize || size > fileSize - static_cast<std::uint64_t>(pos)) { errorText = "Truncated package."; return false; }
            const auto target = directory / Utf8Path(name);
            if (!QFE::PROJECT::IsWithin(target, directory) || std::filesystem::exists(target)) { errorText = "Duplicate or invalid package path."; return false; }
            std::filesystem::create_directories(target.parent_path(), error);
            if (error) { errorText = "Could not create an asset directory."; return false; }
            std::ofstream out(target, std::ios::binary | std::ios::trunc);
            if (!out || !CopyBytes(in, out, size)) { errorText = "Could not extract an asset."; return false; }
        }
        if (in.peek() != std::char_traits<char>::eof() || !std::filesystem::is_regular_file(directory / "project.json") || !std::filesystem::is_directory(directory / "assets")) {
            errorText = "Package has missing or trailing data."; return false;
        }
        return true;
    } catch (const std::exception&) { errorText = "Could not read the package."; return false; }
}
}

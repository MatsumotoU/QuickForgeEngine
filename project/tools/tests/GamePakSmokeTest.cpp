#include "../../engine/assetfactory/game-data/GamePak.h"
#include "../../engine/assetfactory/game-data/AssetRegistry.h"
#include <fstream>
#include <iostream>
#include <iterator>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 2;
    const std::filesystem::path root(argv[1]);
    const auto project = root / "game";
    const auto assets = project / "assets";
    std::filesystem::create_directories(assets / "scenes");
    std::filesystem::create_directories(assets / "entities");
    std::filesystem::create_directories(assets / "images");
    std::filesystem::create_directories(project / "sources" / "models");
    { std::ofstream out(project / "project.json"); out << R"({"name":"test","version":1,"start_scene":"start.scene"})"; }
    { std::ofstream out(assets / "scenes" / "start.scene"); out << "scene"; }
    { std::ofstream out(assets / "entities" / "player.entity"); out << "entity"; }
    { std::ofstream out(project / "asset-registry.json"); out << R"({"version":1,"assets":[{"id":"model-1","type":"model","name":"ship","source":"sources/models/ship.glb","dependencies":[]}]})"; }
    { std::ofstream out(project / "sources" / "models" / "ship.glb"); out << "source"; }
    { std::ofstream out(assets / "images" / "image.bin", std::ios::binary); out << "image bytes"; }
    const std::string binaryData("\x00\x01\xff\x7f\x00\x42", 6);
    { std::ofstream out(assets / "images" / "binary.bin", std::ios::binary); out.write(binaryData.data(), static_cast<std::streamsize>(binaryData.size())); }
    const auto package = root / "game.qfepak";
    const auto extracted = root / "unpacked";
    std::filesystem::create_directory(extracted);
    std::string error;
    const auto check = [](bool ok, const char* message) { if (!ok) std::cerr << message << '\n'; return ok; };
    const auto checkPak = [&](bool passed, const char* message) {
        if (!passed) std::cerr << message << ": " << error << '\n';
        return passed;
    };
    bool ok = true;
    ok &= checkPak(QFE::PAK::Write(project, package, error), "Export failed");
    ok &= checkPak(QFE::PAK::Extract(package, extracted, error), "Extract failed");
    ok &= check(std::filesystem::is_regular_file(extracted / "assets" / "scenes" / "start.scene"), "Scene missing");
    ok &= check(std::filesystem::is_regular_file(extracted / "assets" / "entities" / "player.entity"), "Entity missing");
    ok &= check(std::filesystem::is_regular_file(extracted / "asset-registry.json"), "Registry missing");
    ok &= check(std::filesystem::is_regular_file(extracted / "sources" / "models" / "ship.glb"), "Model source missing");
    QFE::ASSET::AssetRegistry registry;
    ok &= check(registry.Open(extracted, error) && registry.Find("model-1") != nullptr, "Extracted registry could not be opened");
    std::ifstream image(extracted / "assets" / "images" / "image.bin", std::ios::binary);
    std::string text; std::getline(image, text);
    ok &= check(text == "image bytes", "Extracted data changed");
    {
        std::ifstream binary(extracted / "assets" / "images" / "binary.bin", std::ios::binary);
        const std::string decoded{std::istreambuf_iterator<char>(binary), std::istreambuf_iterator<char>()};
        ok &= check(binary.is_open() && decoded == binaryData, "Extracted binary data changed");
    }
    const auto readBytes = [](const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    };
    const auto beforeFailedExport = readBytes(package);
    auto occupiedTemporary = package;
    occupiedTemporary += L".qfe-writing";
    { std::ofstream out(occupiedTemporary, std::ios::binary); out << "existing temporary"; }
    ok &= check(!QFE::PAK::Write(project, package, error), "Export accepted an existing temporary file");
    ok &= check(readBytes(package) == beforeFailedExport && readBytes(occupiedTemporary) == "existing temporary",
        "Failed export modified an existing file");
    std::filesystem::remove(occupiedTemporary);
#ifdef _WIN32
    const HANDLE lockedPackage = CreateFileW(package.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    ok &= check(lockedPackage != INVALID_HANDLE_VALUE, "Could not lock the existing package for testing");
    if (lockedPackage != INVALID_HANDLE_VALUE) {
        const bool replacedWhileLocked = QFE::PAK::Write(project, package, error);
        CloseHandle(lockedPackage);
        ok &= check(!replacedWhileLocked && error.starts_with("Failed to replace the package ("),
            "Locked output did not report a replacement error");
        ok &= check(readBytes(package) == beforeFailedExport && !std::filesystem::exists(occupiedTemporary),
            "Failed replacement changed the existing package or left a temporary file");
    }
#endif
    { std::ofstream out(project / "project.json"); out << R"({"name":"test","version":1,"start_scene":"start"})"; }
    ok &= checkPak(QFE::PAK::Write(project, package, error), "Extensionless .scene export failed");
    std::filesystem::rename(assets / "scenes" / "start.scene", assets / "scenes" / "start.json");
    ok &= checkPak(QFE::PAK::Write(project, package, error), "Legacy extensionless .json export failed");
    std::filesystem::remove(project / "asset-registry.json");
    std::filesystem::remove(project / "sources" / "models" / "ship.glb");
    std::filesystem::remove(project / "sources" / "models");
    std::filesystem::remove(project / "sources");
    ok &= checkPak(QFE::PAK::Write(project, package, error), "Legacy project export failed");
    ok &= check(!QFE::PAK::Write(project, assets / "bad.qfepak", error), "Package inside assets accepted");
    ok &= check(!QFE::PAK::Write(project, project / "sources" / "bad.qfepak", error), "Package inside sources accepted");
    ok &= check(!QFE::PAK::Write(project, project / "project.json", error), "Project metadata could be overwritten");
    ok &= check(!QFE::PAK::ValidName("assets/../outside"), "Traversal accepted");
    ok &= check(!QFE::PAK::ValidName("assets/dir\\outside"), "Backslash accepted");
    ok &= check(!QFE::PAK::ValidName("assets/CON.txt"), "Windows device name accepted");
    const auto legacyPackage = root / "legacy.qfepak";
    {
        std::ofstream out(legacyPackage, std::ios::binary);
        out.write(QFE::PAK::Magic.data(), 8);
        QFE::PAK::WriteNumber(out, 1, 4);
        QFE::PAK::WriteNumber(out, 2, 4);
        const auto entry = [&](const std::string& name, const std::string& body) {
            QFE::PAK::WriteNumber(out, name.size(), 4);
            QFE::PAK::WriteNumber(out, body.size(), 8);
            out.write(name.data(), static_cast<std::streamsize>(name.size()));
            out.write(body.data(), static_cast<std::streamsize>(body.size()));
        };
        entry("project.json", R"({"start_scene":"start.json"})");
        entry("assets/scenes/start.json", "scene");
    }
    const auto legacyRoot = root / "legacy"; std::filesystem::create_directory(legacyRoot);
    ok &= checkPak(QFE::PAK::Extract(legacyPackage, legacyRoot, error), "Legacy version 1 package was rejected");
    ok &= check(std::filesystem::is_regular_file(legacyRoot / "assets/scenes/start.json"), "Legacy scene missing");
    const auto invalidLegacy = root / "invalid-legacy.qfepak";
    {
        std::ofstream out(invalidLegacy, std::ios::binary);
        out.write(QFE::PAK::Magic.data(), 8);
        QFE::PAK::WriteNumber(out, 1, 4);
        QFE::PAK::WriteNumber(out, 1, 4);
        const std::string path = "asset-registry.json";
        QFE::PAK::WriteNumber(out, path.size(), 4);
        QFE::PAK::WriteNumber(out, 0, 8);
        out.write(path.data(), static_cast<std::streamsize>(path.size()));
    }
    const auto invalidLegacyRoot = root / "invalid-legacy"; std::filesystem::create_directory(invalidLegacyRoot);
    ok &= check(!QFE::PAK::Extract(invalidLegacy, invalidLegacyRoot, error), "Version 1 accepted a version 2 path");
    const auto corrupt = root / "truncated.qfepak";
    { std::ofstream out(corrupt, std::ios::binary); out.write(QFE::PAK::Magic.data(), 8); QFE::PAK::WriteNumber(out, 1, 4); QFE::PAK::WriteNumber(out, 1, 4); QFE::PAK::WriteNumber(out, 100, 4); }
    const auto badRoot = root / "bad"; std::filesystem::create_directory(badRoot);
    ok &= check(!QFE::PAK::Extract(corrupt, badRoot, error), "Truncated package accepted");
    return ok ? 0 : 1;
}

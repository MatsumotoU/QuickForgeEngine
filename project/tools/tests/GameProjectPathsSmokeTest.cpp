#include "../../engine/assetfactory/game-data/GameProjectPaths.h"
#include <filesystem>
#include <iostream>

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 2;
    const auto base = std::filesystem::absolute(argv[1]);
    const auto first = base / L"project-a";
    const auto second = base / L"project-b";
    const auto scenes = first / L"assets" / L"scenes";
    const auto check = [](bool condition, const char* message) {
        if (!condition) std::cerr << message << '\n';
        return condition;
    };
    QFE::PROJECT::SetRoot(first);
    bool passed = true;
    passed &= check(QFE::PROJECT::Assets() == first / L"assets", "Wrong assets root");
    passed &= check(QFE::PROJECT::IsWithin(scenes / L"same.json", scenes), "Own scene rejected");
    passed &= check(QFE::PROJECT::IsWithin(first / L"ASSETS" / L"SCENES" / L"same.json", scenes), "Case-insensitive path rejected");
    passed &= check(!QFE::PROJECT::IsWithin(second / L"assets" / L"scenes" / L"same.json", scenes), "Other project accepted");
    passed &= check(!QFE::PROJECT::IsWithin(scenes / L".." / L".." / L".." / L".." / L"project-b" / L"assets" / L"scenes" / L"same.json", scenes), "Parent traversal accepted");
    passed &= check(!QFE::PROJECT::IsWithin(base / L"project-a" / L"assets" / L"scenes-other" / L"same.json", scenes), "Sibling directory accepted");
    passed &= check(QFE::PROJECT::Resolve(L"resources/scene/same.json") == scenes / L"same.json", "Legacy scene path resolved incorrectly");
    QFE::PROJECT::SetRoot({});
    return passed ? 0 : 1;
}

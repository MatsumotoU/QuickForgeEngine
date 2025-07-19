#pragma once
#include <string>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

#include "FontStructures.h" // 上記で定義した構造体

class FontDataLoader
{
public:
    FontAtlasData LoadFontData(const std::string& jsonFilePath);
};
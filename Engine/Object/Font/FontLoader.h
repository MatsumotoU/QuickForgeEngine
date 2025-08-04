#pragma once
#include <string>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

#include "FontStructures.h" // 上記で定義した構造体

namespace FontLoader
{
	// JSONファイルからフォントデータを読み込む関数
	FontAtlasData LoadFontData(const std::string& jsonFilePath);
	// フォントデータをシェーダー用の構造体に変換する関数
	FontDataToShader ConvertToShaderData(const FontAtlasData& fontAtlasData);
	// グリフのUVデータを取得する関数
	FontUVData GetGlyphUVData(const FontAtlasData& fontAtlasData, unsigned int unicode);
} // namespace FontLoader
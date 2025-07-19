#include "FontLoader.h"

FontAtlasData FontLoader::LoadFontData(const std::string& jsonFilePath) {
    FontAtlasData fontData;
    std::ifstream file(jsonFilePath);
    if (!file.is_open())
    {
        // エラー処理
        throw std::runtime_error("Failed to open JSON file: " + jsonFilePath);
    }

    nlohmann::json j;
    file >> j;

    // --- atlas情報 ---
    fontData.type = j["atlas"]["type"].get<std::string>();
    fontData.distanceRange = j["atlas"]["distanceRange"].get<float>();
    fontData.width = j["atlas"]["width"].get<float>();
    fontData.height = j["atlas"]["height"].get<float>();
    fontData.yOrigin = j["atlas"]["yOrigin"].get<std::string>();

    // --- metrics情報 ---
    fontData.metrics.emSize = j["metrics"]["emSize"].get<float>();
    fontData.metrics.lineHeight = j["metrics"]["lineHeight"].get<float>();
    fontData.metrics.ascender = j["metrics"]["ascender"].get<float>();
    fontData.metrics.descender = j["metrics"]["descender"].get<float>();
    // 他のmetricsもパース

    // --- glyphs情報 ---
    for (const auto& glyphJson : j["glyphs"])
    {
        GlyphInfo glyph;
        glyph.unicode = glyphJson["unicode"].get<unsigned int>();
        glyph.advance = glyphJson["advance"].get<float>();

        glyph.planeBounds.left = glyphJson["planeBounds"]["left"].get<float>();
        glyph.planeBounds.bottom = glyphJson["planeBounds"]["bottom"].get<float>();
        glyph.planeBounds.right = glyphJson["planeBounds"]["right"].get<float>();
        glyph.planeBounds.top = glyphJson["planeBounds"]["top"].get<float>();

        glyph.atlasBounds.left = glyphJson["atlasBounds"]["left"].get<float>();
        glyph.atlasBounds.bottom = glyphJson["atlasBounds"]["bottom"].get<float>();
        glyph.atlasBounds.right = glyphJson["atlasBounds"]["right"].get<float>();
        glyph.atlasBounds.top = glyphJson["atlasBounds"]["top"].get<float>();

        fontData.glyphs[glyph.unicode] = glyph;
    }

    return fontData;
}

FontDataToShader FontLoader::ConvertToShaderData(const FontAtlasData& fontAtlasData) {
    FontDataToShader result{};
    result.AtlasSize = Vector2(fontAtlasData.width, fontAtlasData.height);
    result.DistanceRange = fontAtlasData.distanceRange;
	result.padding = 0.0f; // 必要に応じてパディングを設定
    return result;
}

FontUVData FontLoader::GetGlyphUVData(const FontAtlasData& fontAtlasData, unsigned int unicode) {
	auto it = fontAtlasData.glyphs.find(unicode);
	if (it != fontAtlasData.glyphs.end()) {
		const GlyphInfo& glyph = it->second;
        FontUVData uvData{};
		uvData.leftTop = Vector2(glyph.atlasBounds.left / fontAtlasData.width, glyph.atlasBounds.top / fontAtlasData.height);
		uvData.rightTop = Vector2(glyph.atlasBounds.right / fontAtlasData.width, glyph.atlasBounds.top / fontAtlasData.height);
		uvData.leftBottom = Vector2(glyph.atlasBounds.left / fontAtlasData.width, glyph.atlasBounds.bottom / fontAtlasData.height);
		uvData.rightBottom = Vector2(glyph.atlasBounds.right / fontAtlasData.width, glyph.atlasBounds.bottom / fontAtlasData.height);
		return uvData;
	}

	// グリフが見つからない場合はデフォルトのUVデータを返す
	assert(false && "Glyph not found in font atlas data");
    return FontUVData();
}

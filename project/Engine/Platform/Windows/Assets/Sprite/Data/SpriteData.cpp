#include "SpriteData.h"

SpriteData::SpriteData() {
	textureName = "";
	textureHandle = 0;
	vertexBufferHandle = 0;
	wvpBufferHandle = 0;
	materialBufferHandle = 0;
	lightBufferHandle = 0;
	layer = 0;
	width = 100.0f;
	height = 100.0f;
	pivot = { 0.0f,0.0f };
	isDraw = true;
}

nlohmann::json SpriteData::Serialize() const {
	nlohmann::json json;
	json["textureName"] = textureName;
	json["width"] = width;
	json["height"] = height;
	json["layer"] = layer;
	json["pivot"] = { pivot.x, pivot.y };
	json["isDraw"] = isDraw;
	json["color"] = { color.x, color.y, color.z, color.w };	
	return json;
}

void SpriteData::Deserialize(const nlohmann::json& json) {
	if (json.contains("textureName")) {
		textureName = json["textureName"].get<std::string>();
	}
	if (json.contains("width")) {
		width = json["width"].get<float>();
	}
	if (json.contains("height")) {
		height = json["height"].get<float>();
	}
	if (json.contains("layer")) {
		layer = json["layer"].get<uint32_t>();
	}
	if (json.contains("pivot")) {
		pivot.x = json["pivot"][0].get<float>();
		pivot.y = json["pivot"][1].get<float>();
	}

	if (json.contains("isDraw")) {
		isDraw = json["isDraw"].get<bool>();
	}
	if (json.contains("color")) {
		color.x = json["color"][0].get<float>();
		color.y = json["color"][1].get<float>();
		color.z = json["color"][2].get<float>();
		color.w = json["color"][3].get<float>();
	}
}

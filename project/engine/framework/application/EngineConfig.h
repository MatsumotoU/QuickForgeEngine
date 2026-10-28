#pragma once
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

namespace QFE::FRAMEWORK {
	struct EngineConfig {
		static constexpr const char* path = "engine/resources/EngineConfig.json";
		uint32_t directionalLightMaxNum = 16;
		uint32_t particleMaxNum = 65536;
		uint32_t windowWidth = 1280;
		uint32_t windowHeight = 720;
		std::string editorWindowName = "ShootingGameEditor";
		std::string runtimeWindowName = "ShootingGameRuntime";
		std::string assetDir = "resources/";
		std::string modelDir = "resources/";
		std::string scriptDir = "resources/Scripts";
		std::string psDirName = "engine/resources/shaders/ps/";
		std::string vsDirName = "engine/resources/shaders/vs/";
		std::string csDirName = "engine/resources/shaders/cs/";
		std::string rtDirName = "engine/resources/shaders/rt/";

		bool Load() {
			std::ifstream file(path);
			if (!file) return false;
			try {
				const auto json = nlohmann::json::parse(file);
				if (!json.is_object()) return false;
				auto number = [&](const char* key, uint32_t& target, uint32_t max) {
					auto it = json.find(key);
					if (it != json.end() && it->is_number_integer()) {
						const auto value = it->get<int64_t>();
						target = static_cast<uint32_t>((std::clamp)(value, int64_t{1}, static_cast<int64_t>(max)));
					}
				};
				auto str = [&](const char* key, std::string& target) {
					auto it = json.find(key);
					if (it != json.end() && it->is_string() && !it->get<std::string>().empty()) target = it->get<std::string>();
				};
				number("DirectionalLightMaxNum", directionalLightMaxNum, 4096);
				number("ParticleMaxNum", particleMaxNum, 1048576);
				number("WindowWidth", windowWidth, 8192);
				number("WindowHeight", windowHeight, 8192);
				str("EditorWindowName", editorWindowName); str("RuntimeWindowName", runtimeWindowName);
				str("AssetDir", assetDir); str("ModelDir", modelDir);
				str("ScriptDir", scriptDir);
				str("PixelShaderDir", psDirName); str("VertexShaderDir", vsDirName);
				str("ComputeShaderDir", csDirName); str("RayTracingShaderDir", rtDirName);
				auto normalizeDir = [](std::string& dir) {
					if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';
				};
				normalizeDir(assetDir); normalizeDir(modelDir); normalizeDir(psDirName);
				normalizeDir(vsDirName); normalizeDir(csDirName); normalizeDir(rtDirName);
				return true;
			} catch (const nlohmann::json::exception&) { return false; }
		}
		bool Save() const {
			nlohmann::json json = nlohmann::json::object();
			std::ifstream existing(path);
			if (existing) {
				try { json = nlohmann::json::parse(existing); } catch (const nlohmann::json::exception&) { return false; }
				if (!json.is_object()) return false;
			}
			existing.close();
			json["DirectionalLightMaxNum"] = directionalLightMaxNum; json["ParticleMaxNum"] = particleMaxNum;
			json["WindowWidth"] = windowWidth; json["WindowHeight"] = windowHeight;
			json["EditorWindowName"] = editorWindowName; json["RuntimeWindowName"] = runtimeWindowName;
			json["AssetDir"] = assetDir; json["ModelDir"] = modelDir;
			json["ScriptDir"] = scriptDir;
			json["PixelShaderDir"] = psDirName; json["VertexShaderDir"] = vsDirName;
			json["ComputeShaderDir"] = csDirName; json["RayTracingShaderDir"] = rtDirName;
			std::ofstream file(path, std::ios::trunc);
			if (!file) return false;
			file << json.dump(4) << '\n';
			return file.good();
		}
	};
}

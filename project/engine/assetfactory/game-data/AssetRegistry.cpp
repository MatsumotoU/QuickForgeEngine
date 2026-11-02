#include "AssetRegistry.h"
#include "GameProjectPaths.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace {
bool ValidAsset(const QFE::ASSET::AssetMetadata& asset, const std::filesystem::path& root) {
	if (asset.id.empty() || asset.type.empty() || asset.name.empty() || asset.source.empty()) return false;
	if (!std::all_of(asset.id.begin(), asset.id.end(), [](unsigned char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '-' || c == '_';
	})) return false;
	const std::filesystem::path source(std::u8string(reinterpret_cast<const char8_t*>(asset.source.data()), asset.source.size()));
	if (source.is_absolute() || source.has_root_name() || source.has_root_directory()) return false;
	for (const auto& part : source) if (part == "." || part == "..") return false;
	return QFE::PROJECT::IsWithin(root / source, root);
}
}

bool QFE::ASSET::AssetRegistry::Open(const std::filesystem::path& projectRoot, std::string& error) {
	error.clear();
	std::error_code ec;
	const auto root = std::filesystem::absolute(projectRoot, ec);
	if (projectRoot.empty() || ec || !std::filesystem::is_directory(root, ec) || ec) {
		error = "Invalid project directory.";
		return false;
	}
	std::vector<AssetMetadata> loaded;
	const auto path = root / "asset-registry.json";
	const bool exists = std::filesystem::exists(path, ec);
	if (ec) { error = "Could not check the asset registry."; return false; }
	if (exists) {
		try {
			std::ifstream input(path, std::ios::binary);
			if (!input) { error = "Could not open the asset registry."; return false; }
			const auto json = nlohmann::json::parse(input);
			if (json.at("version") != 1 || !json.at("assets").is_array()) {
				error = "Unsupported asset registry format."; return false;
			}
			for (const auto& item : json.at("assets")) {
				AssetMetadata entry{item.at("id").get<std::string>(), item.at("type").get<std::string>(),
					item.at("name").get<std::string>(), item.at("source").get<std::string>(),
					item.value("dependencies", std::vector<std::string>{}), item.value("previous_sources", std::vector<std::string>{})};
				if (!ValidAsset(entry, root) || std::any_of(loaded.begin(), loaded.end(),
					[&](const auto& other) { return other.id == entry.id; })) {
					error = "Invalid asset entry or duplicate ID."; return false;
				}
				loaded.push_back(std::move(entry));
			}
		} catch (const std::exception&) { error = "Could not parse the asset registry."; return false; }
	}
	root_ = root;
	assets_ = std::move(loaded);
	return true;
}

bool QFE::ASSET::AssetRegistry::Register(const AssetMetadata& asset, std::string& error) {
	error.clear();
	if (root_.empty() || !ValidAsset(asset, root_) || Find(asset.id)) {
		error = "Invalid asset entry or duplicate ID."; return false;
	}
	assets_.push_back(asset);
	return true;
}

bool QFE::ASSET::AssetRegistry::Update(const AssetMetadata& asset, std::string& error) {
	error.clear();
	if (root_.empty() || !ValidAsset(asset, root_)) { error = "Invalid asset entry."; return false; }
	const auto it = std::find_if(assets_.begin(), assets_.end(), [&](const auto& entry) { return entry.id == asset.id; });
	if (it == assets_.end()) { error = "Asset ID was not found."; return false; }
	*it = asset;
	return true;
}

const QFE::ASSET::AssetMetadata* QFE::ASSET::AssetRegistry::Find(const std::string& id) const {
	const auto it = std::find_if(assets_.begin(), assets_.end(), [&](const auto& asset) { return asset.id == id; });
	return it == assets_.end() ? nullptr : &*it;
}

bool QFE::ASSET::AssetRegistry::Save(std::string& error) const {
	error.clear();
	if (root_.empty()) { error = "No project is open."; return false; }
	nlohmann::json json = {{"version", 1}, {"assets", nlohmann::json::array()}};
	for (const auto& entry : assets_) {
		json["assets"].push_back({{"id", entry.id}, {"type", entry.type}, {"name", entry.name},
			{"source", entry.source}, {"dependencies", entry.dependencies}, {"previous_sources", entry.previousSources}});
	}
	const auto target = root_ / "asset-registry.json";
	const auto temporary = root_ / "asset-registry.json.qfe-writing";
	std::error_code ec;
	if (std::filesystem::exists(temporary, ec) || ec) { error = "Registry temporary file already exists or cannot be checked."; return false; }
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output) { error = "Could not create the registry."; return false; }
	output << json.dump(4) << '\n';
	output.close();
	if (!output) { std::filesystem::remove(temporary, ec); error = "Could not write the registry."; return false; }
#ifdef _WIN32
	const bool replaced = MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
#else
	std::filesystem::rename(temporary, target, ec);
	const bool replaced = !ec;
#endif
	if (!replaced) { std::filesystem::remove(temporary, ec); error = "Could not replace the registry."; return false; }
	return true;
}

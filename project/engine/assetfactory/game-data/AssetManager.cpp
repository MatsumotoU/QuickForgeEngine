#include "AssetManager.h"
#include "assetfactory/model/AssimpModelLoader.h"
#include "GameProjectPaths.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {
	std::string ImageExtension(const std::filesystem::path& path) {
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return ext;
	}
	std::filesystem::path Utf8Path(const std::string& text) {
		return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
	}
	std::string TrimPath(std::string text) {
		const auto first = text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return {};
		const auto last = text.find_last_not_of(" \t\r\n");
		text = text.substr(first, last - first + 1);
		if (text.size() >= 2 && text.front() == '"' && text.back() == '"') text = text.substr(1, text.size() - 2);
		return text;
	}
	bool ParseMtlTexture(const std::string& text, std::string& options, std::string& path, std::string& error) {
		size_t position = 0;
		const auto skipSpace = [&]() { while (position < text.size() && (text[position] == ' ' || text[position] == '\t')) ++position; };
		const auto token = [&]() {
			skipSpace();
			const size_t start = position;
			while (position < text.size() && text[position] != ' ' && text[position] != '\t' && text[position] != '\r') ++position;
			return text.substr(start, position - start);
		};
		skipSpace();
		while (position < text.size() && text[position] == '-') {
			const auto option = token();
			int minimum = 1, maximum = 1;
			bool numeric = false;
			if (option == "-o" || option == "-s" || option == "-t") { maximum = 3; numeric = true; }
			else if (option == "-mm") { minimum = maximum = 2; numeric = true; }
			else if (option == "-bm" || option == "-boost" || option == "-texres") numeric = true;
			else if (option != "-blendu" && option != "-blendv" && option != "-clamp" &&
				option != "-imfchan" && option != "-type" && option != "-cc") {
				error = "Unsupported MTL texture option: " + option; return false;
			}
			int count = 0;
			for (; count < maximum; ++count) {
				const size_t before = position;
				const auto argument = token();
				bool valid = !argument.empty();
				if (valid && numeric) {
					char* end = nullptr;
					std::strtod(argument.c_str(), &end);
					valid = end != argument.c_str() && *end == '\0';
				}
				if (!valid) { position = before; break; }
			}
			if (count < minimum) { error = "Invalid MTL texture option: " + option; return false; }
			skipSpace();
		}
		options = text.substr(0, position);
		path = TrimPath(text.substr(position));
		if (path.empty()) { error = "MTL texture path is empty."; return false; }
		return true;
	}
	bool ValidateCopiedModel(const std::filesystem::path& path, std::string& error) {
		const auto text = path.generic_u8string();
		const std::string utf8(reinterpret_cast<const char*>(text.data()), text.size());
		QFE::ASSET::AssimpModelLoader loader;
		loader.Initialize();
		QFE::ASSET::ModelData data;
		if (!loader.LoadModel(utf8, data) || data.meshes.empty()) { error = "Could not load the imported model."; return false; }
		for (const auto& mesh : data.meshes.GetInternalVector()) {
			const auto& material = mesh.material;
			if (material.textureName.empty() || !material.embeddedTextureData.empty()) continue;
			std::error_code ec;
			if (!std::filesystem::is_regular_file(Utf8Path(material.textureName), ec) || ec) {
				error = "The imported model refers to a missing texture."; return false;
			}
		}
		return true;
	}
}

bool QFE::ASSET::AssetManager::ImportRawFile(const std::filesystem::path& source, std::string& error) {
	error.clear();
	std::error_code ec;
	if (root_.empty() || !std::filesystem::is_regular_file(source, ec) || ec) { error = "Source file was not found."; return false; }
	const auto ext = ImageExtension(source);
	const char* category = "other";
	if (ext == ".obj" || ext == ".glb") category = "models";
	else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tif" || ext == ".tiff" || ext == ".dds" || ext == ".tga") category = "images";
	else if (ext == ".wav" || ext == ".mp3") category = "audio";
	else if (ext == ".lua") category = "scripts";
	else if (ext == ".scene") category = "scenes";
	else if (ext == ".entity") category = "entities";
	else if (ext == ".anim") category = "animations";
	else if (ext == ".hlsl" || ext == ".hlsli" || ext == ".cso") category = "shaders";
	const auto assets = root_ / "assets";
	if (QFE::PROJECT::IsWithin(source, assets)) return true;
	auto directory = assets / category;
	// OBJの関連ファイルはモデルごとのディレクトリに元の相対パスで保存する。
	if (ext == ".obj") directory /= source.stem();
	const auto sourceBase = ext == ".obj" ? source.parent_path().parent_path() : source.parent_path();
	const auto modelDirectory = ext == ".obj" ? directory / source.parent_path().filename() : directory;
	const auto destination = modelDirectory / source.filename();
	if (!QFE::PROJECT::IsWithin(destination, assets)) { error = "Invalid asset destination."; return false; }
	std::vector<std::filesystem::path> created;
	auto copy = [&](const std::filesystem::path& from, const std::filesystem::path& to) {
		if (!QFE::PROJECT::IsWithin(from, sourceBase) || !QFE::PROJECT::IsWithin(to, directory)) { error = "A model dependency is outside its source directory."; return false; }
		if (std::find(created.begin(), created.end(), to) != created.end()) return true;
		std::filesystem::create_directories(to.parent_path(), ec);
		if (ec || !std::filesystem::copy_file(from, to, std::filesystem::copy_options::none, ec) || ec) { error = "Could not copy asset; the file may already exist or a dependency may be missing."; return false; }
		created.push_back(to); return true;
	};
	bool ok = copy(source, destination);
	if (ok && ext == ".obj") {
		std::ifstream obj(source, std::ios::binary);
		std::string line;
		while (ok && std::getline(obj, line)) {
			std::istringstream words(line); std::string tag; words >> tag;
			if (tag != "mtllib") continue;
			std::string rest; std::getline(words, rest);
			std::vector<std::string> names;
			const auto whole = TrimPath(rest);
			if (!whole.empty() && std::filesystem::is_regular_file(source.parent_path() / Utf8Path(whole), ec) && !ec) names.push_back(whole);
			else { ec.clear(); std::istringstream list(rest); std::string name; while (list >> std::quoted(name)) names.push_back(name); }
			if (names.empty()) { error = "OBJ material file is missing."; ok = false; break; }
			for (const auto& name : names) {
				const auto relative = Utf8Path(name);
				if (relative.is_absolute() || relative.has_root_name()) { error = "OBJ material path must be relative."; ok = false; break; }
				const auto material = (source.parent_path() / relative).lexically_normal();
				const auto target = (modelDirectory / relative).lexically_normal();
				if (!copy(material, target)) { ok = false; break; }
				std::ifstream input(material, std::ios::binary);
				std::string materialLine;
				while (ok && std::getline(input, materialLine)) {
					std::istringstream parts(materialLine); std::string kind; parts >> kind;
					std::transform(kind.begin(), kind.end(), kind.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					if (!kind.starts_with("map_") && kind != "bump" && kind != "disp" && kind != "decal" && kind != "norm") continue;
					std::string spec, options, textureName; std::getline(parts, spec);
					if (!ParseMtlTexture(spec, options, textureName, error)) { ok = false; break; }
					const auto texture = Utf8Path(textureName);
					if (texture.is_absolute() || texture.has_root_name()) { error = "MTL texture path must be relative."; ok = false; break; }
					if (!copy((material.parent_path() / texture).lexically_normal(), (target.parent_path() / texture).lexically_normal())) { ok = false; break; }
				}
				if (ok && !input.eof()) { error = "Could not read OBJ material."; ok = false; }
			}
		}
		if (ok && !obj.eof()) { error = "Could not read OBJ model."; ok = false; }
	}
	if (ok && (ext == ".obj" || ext == ".glb")) ok = ValidateCopiedModel(destination, error);
	if (!ok) { for (const auto& file : created) { std::error_code ignored; std::filesystem::remove(file, ignored); } }
	return ok;
}


bool QFE::ASSET::AssetManager::Open(const std::filesystem::path& projectRoot, std::string& error) {
	error.clear();
	std::error_code ec;
	const auto root = std::filesystem::absolute(projectRoot, ec);
	if (projectRoot.empty() || ec || !std::filesystem::is_directory(root, ec) || ec) { error = "Invalid project directory."; return false; }
	root_ = root;
	return true;
}

std::vector<std::string> QFE::ASSET::AssetManager::ListProjectFiles(std::string& error) const {
	error.clear();
	std::vector<std::string> files;
	if (root_.empty()) { error = "No project is open."; return files; }
	const auto assets = root_ / "assets";
	std::error_code ec;
	for (std::filesystem::recursive_directory_iterator it(assets, ec), end; !ec && it != end; it.increment(ec)) {
		const auto status = it->symlink_status(ec);
		if (ec) break;
		if (!std::filesystem::is_regular_file(status)) continue;
		const auto path = it->path().lexically_relative(assets).generic_u8string();
		files.emplace_back(reinterpret_cast<const char*>(path.data()), path.size());
	}
	if (ec) { files.clear(); error = "Could not enumerate project assets."; return files; }
	std::sort(files.begin(), files.end());
	return files;
}

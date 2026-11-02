#include "AssetManager.h"
#include "assetfactory/model/PrimitiveFactoryFuncs.h"
#include "GameProjectPaths.h"
#include "ecs/component/EntityUuid.h"
#include <algorithm>
#include <cctype>
#include <limits>
#include <fstream>
#include <sstream>

namespace {
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
	bool CopyObjDependencies(const std::filesystem::path& source, const std::filesystem::path& directory, std::string& error) {
		std::ifstream input(source, std::ios::binary);
		std::ofstream output(directory / "model.obj", std::ios::binary);
		if (!input || !output) { error = "Could not read or write the OBJ file."; return false; }
		std::string line;
		unsigned materialNumber = 0;
		unsigned textureNumber = 0;
		while (std::getline(input, line)) {
			std::istringstream words(line);
			std::string tag;
			words >> tag;
			if (tag != "mtllib") { output << line << '\n'; continue; }
			std::string remainder;
			std::getline(words, remainder);
			const auto materialName = TrimPath(remainder);
			const auto materialPath = Utf8Path(materialName);
			if (materialName.empty() || materialPath.is_absolute() || materialPath.has_root_name()) {
				error = "OBJ contains an invalid material path."; return false;
			}
			const auto materialFile = (source.parent_path() / materialPath).lexically_normal();
			std::ifstream material(materialFile, std::ios::binary);
			if (!material) { error = "OBJ material file could not be read: " + materialName; return false; }
			const std::string newName = "material-" + std::to_string(materialNumber++) + ".mtl";
			std::ofstream copied(directory / newName, std::ios::binary);
			if (!copied) { error = "Could not write the material file."; return false; }
			std::string materialLine;
			while (std::getline(material, materialLine)) {
				std::istringstream materialWords(materialLine);
				std::string materialTag;
				materialWords >> materialTag;
				std::string lower = materialTag;
				std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (!lower.starts_with("map_") && lower != "bump" && lower != "disp" && lower != "decal" && lower != "norm") {
					copied << materialLine << '\n'; continue;
				}
				std::string textureName;
				std::getline(materialWords, textureName);
				textureName = TrimPath(textureName);
				if (textureName.empty() || textureName.front() == '-') {
					error = "MTL texture options are not supported for import."; return false;
				}
				const auto relativeTexture = Utf8Path(textureName);
				if (relativeTexture.is_absolute() || relativeTexture.has_root_name()) { error = "MTL contains an absolute texture path."; return false; }
				const auto texture = (materialFile.parent_path() / relativeTexture).lexically_normal();
				std::error_code ec;
				if (!std::filesystem::is_regular_file(texture, ec) || ec) { error = "MTL texture was not found: " + textureName; return false; }
				std::filesystem::create_directories(directory / "textures", ec);
				if (ec) { error = "Could not create the model texture directory."; return false; }
				const auto targetName = "texture-" + std::to_string(textureNumber++) + texture.extension().string();
				if (!std::filesystem::copy_file(texture, directory / "textures" / targetName, std::filesystem::copy_options::none, ec) || ec) {
					error = "Could not copy the model texture."; return false;
				}
				copied << materialTag << " textures/" << targetName << '\n';
			}
			copied.close();
			if (!material.eof() || !copied) { error = "Could not copy the material file."; return false; }
			output << "mtllib " << newName << '\n';
		}
		output.close();
		if (!input.eof() || !output) { error = "Could not copy the OBJ file."; return false; }
		return true;
	}
}

std::string QFE::ASSET::AssetManager::ImportGlb(const std::filesystem::path& source, std::string& error) {
	error.clear();
	std::error_code ec;
	if (root_.empty() || !std::filesystem::is_regular_file(source, ec) || ec) {
		error = "Model file was not found."; return {};
	}
	std::string extension = source.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (extension != ".glb") { error = "Only GLB models can be imported currently."; return {}; }
	const auto sourceText = source.generic_u8string();
	const std::string sourceUtf8(reinterpret_cast<const char*>(sourceText.data()), sourceText.size());
	AssimpModelLoader validator;
	validator.Initialize();
	ModelData data;
	if (!validator.LoadModel(sourceUtf8, data) || data.meshes.empty()) {
		error = "Could not import the GLB model."; return {};
	}
	const std::string id = QFE::GenerateEntityUuid();
	const auto directory = root_ / "sources" / "models";
	const auto destination = directory / (id + ".glb");
	if (!QFE::PROJECT::IsWithin(destination, root_) || !QFE::PROJECT::IsWithin(destination, root_ / "sources")) {
		error = "Invalid model destination."; return {};
	}
	std::filesystem::create_directories(directory, ec);
	if (ec || !std::filesystem::copy_file(source, destination, std::filesystem::copy_options::none, ec) || ec) {
		error = "Could not copy the model into the project."; return {};
	}
	if (!ValidateCopiedModel(destination, error)) { std::filesystem::remove(destination, ec); return {}; }
	const auto filename = source.filename().generic_u8string();
	AssetMetadata entry{id, "model", std::string(reinterpret_cast<const char*>(filename.data()), filename.size()),
		"sources/models/" + id + ".glb", {}, {}};
	AssetRegistry candidate = registry_;
	if (!candidate.Register(entry, error) || !candidate.Save(error)) {
		std::filesystem::remove(destination, ec);
		return {};
	}
	registry_ = std::move(candidate);
	return id;
}

std::string QFE::ASSET::AssetManager::ImportObj(const std::filesystem::path& source, std::string& error) {
	error.clear();
	std::error_code ec;
	if (root_.empty() || !std::filesystem::is_regular_file(source, ec) || ec) { error = "Model file was not found."; return {}; }
	std::string extension = source.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (extension != ".obj") { error = "Expected an OBJ model."; return {}; }
	const std::string id = QFE::GenerateEntityUuid();
	const auto directory = root_ / "sources" / "models" / id;
	if (!QFE::PROJECT::IsWithin(directory, root_) || !QFE::PROJECT::IsWithin(directory, root_ / "sources")) {
		error = "Invalid model destination."; return {};
	}
	std::filesystem::create_directories(directory.parent_path(), ec);
	if (ec || !std::filesystem::create_directory(directory, ec) || ec) { error = "Could not create the model directory."; return {}; }
	// 作成したディレクトリだけを失敗時に片付ける。
	auto cleanup = [&]() { std::error_code ignored; std::filesystem::remove_all(directory, ignored); };
	if (!CopyObjDependencies(source, directory, error)) { cleanup(); return {}; }
	if (!ValidateCopiedModel(directory / "model.obj", error)) { cleanup(); return {}; }
	const auto filename = source.filename().generic_u8string();
	AssetMetadata entry{id, "model", std::string(reinterpret_cast<const char*>(filename.data()), filename.size()),
		"sources/models/" + id + "/model.obj", {}, {}};
	AssetRegistry candidate = registry_;
	if (!candidate.Register(entry, error) || !candidate.Save(error)) { cleanup(); return {}; }
	registry_ = std::move(candidate);
	return id;
}

bool QFE::ASSET::AssetManager::ReplaceModel(const std::string& id, const std::filesystem::path& source, std::string& error) {
	error.clear();
	const auto* current = registry_.Find(id);
	if (!current || current->type != "model") { error = "Model asset was not found."; return false; }
	std::string currentExtension = std::filesystem::path(current->source).extension().string();
	std::string extension = source.extension().string();
	for (auto* text : { &currentExtension, &extension }) {
		std::transform(text->begin(), text->end(), text->begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	}
	if ((extension != ".glb" && extension != ".obj") || currentExtension != extension) {
		error = "Replace the model with a file of the same type (GLB or OBJ)."; return false;
	}
	std::error_code ec;
	if (!std::filesystem::is_regular_file(source, ec) || ec) { error = "Model file was not found."; return false; }
	const auto previous = models_.find(id);
	if (previous != models_.end() && previous->second->revision == (std::numeric_limits<uint64_t>::max)()) {
		error = "Model revision overflow."; return false;
	}
	const auto base = root_ / "sources" / "models";
	const auto unique = QFE::GenerateEntityUuid();
	const auto directory = base / unique;
	const auto destination = extension == ".obj" ? directory / "model.obj" : base / (unique + ".glb");
	if (!QFE::PROJECT::IsWithin(destination, root_) || !QFE::PROJECT::IsWithin(destination, root_ / "sources")) {
		error = "Invalid model destination."; return false;
	}
	std::filesystem::create_directories(base, ec);
	if (ec) { error = "Could not create the model directory."; return false; }
	if (extension == ".obj") {
		if (!std::filesystem::create_directory(directory, ec) || ec) { error = "Could not create the model directory."; return false; }
		if (!CopyObjDependencies(source, directory, error)) { std::filesystem::remove_all(directory, ec); return false; }
	} else if (!std::filesystem::copy_file(source, destination, std::filesystem::copy_options::none, ec) || ec) {
		error = "Could not copy the replacement model."; return false;
	}
	auto cleanup = [&]() {
		std::error_code ignored;
		if (extension == ".obj") std::filesystem::remove_all(directory, ignored);
		else std::filesystem::remove(destination, ignored);
	};
	AssetMetadata updated = *current;
	if (current->source.starts_with("assets/") &&
		std::find(updated.previousSources.begin(), updated.previousSources.end(), current->source) == updated.previousSources.end()) {
		updated.previousSources.push_back(current->source);
	}
	const auto relative = destination.lexically_relative(root_).generic_u8string();
	updated.source.assign(reinterpret_cast<const char*>(relative.data()), relative.size());
	AssetRegistry candidate = registry_;
	if (!candidate.Update(updated, error)) { cleanup(); return false; }
	AssetManager staged;
	staged.root_ = root_;
	staged.registry_ = candidate;
	const auto loaded = staged.LoadModel(id, error);
	if (!loaded) { cleanup(); return false; }
	if (!ValidateCopiedModel(destination, error)) { cleanup(); return false; }
	auto replacement = std::make_shared<ModelAsset>(*loaded);
	replacement->revision = previous == models_.end() ? 1 : previous->second->revision + 1;
	if (!candidate.Save(error)) { cleanup(); return false; }
	registry_ = std::move(candidate);
	models_[id] = replacement;
	return true;
}

bool QFE::ASSET::AssetManager::RegisterProjectFiles(std::string& error) {
	error.clear();
	if (root_.empty()) { error = "No project is open."; return false; }
	AssetRegistry candidate = registry_;
	bool changed = false;
	std::error_code ec;
	const auto assetsRoot = root_ / "assets";
	for (std::filesystem::recursive_directory_iterator it(assetsRoot, ec), end; !ec && it != end; it.increment(ec)) {
		const auto status = it->symlink_status(ec);
		if (ec) break;
		if (!std::filesystem::is_regular_file(status)) continue;
		std::string extension = it->path().extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (extension == ".mtl") continue;
		const auto path = it->path().lexically_relative(root_).generic_u8string();
		const std::string source(reinterpret_cast<const char*>(path.data()), path.size());
		if (std::any_of(candidate.List().begin(), candidate.List().end(),
			[&](const auto& entry) { return entry.source == source ||
				std::find(entry.previousSources.begin(), entry.previousSources.end(), source) != entry.previousSources.end(); })) continue;
		std::string type = "other";
		if (extension == ".obj" || extension == ".glb") type = "model";
		else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".dds" || extension == ".tga" || extension == ".bmp" || extension == ".tif" || extension == ".tiff") type = "image";
		else if (extension == ".scene" || (extension == ".json" && source.starts_with("assets/scenes/"))) type = "scene";
		else if (extension == ".entity" || (extension == ".json" && source.starts_with("assets/entities/"))) type = "entity";
		else if (extension == ".anim") type = "animation";
		else if (extension == ".lua") type = "script";
		else if (extension == ".mp3" || extension == ".wav") type = "audio";
		else if (extension == ".hlsl" || extension == ".hlsli" || extension == ".cso") type = "shader";
		if (type == "other") continue;
		const auto filename = it->path().filename().generic_u8string();
		AssetMetadata entry{QFE::GenerateEntityUuid(), type,
			std::string(reinterpret_cast<const char*>(filename.data()), filename.size()), source, {}, {}};
		if (!candidate.Register(entry, error)) return false;
		changed = true;
	}
	if (ec) { error = "Could not enumerate project assets."; return false; }
	if (changed && !candidate.Save(error)) return false;
	registry_ = std::move(candidate);
	return true;
}

bool QFE::ASSET::AssetManager::Open(const std::filesystem::path& projectRoot, std::string& error) {
	AssetRegistry registry;
	if (!registry.Open(projectRoot, error)) return false;
	std::error_code ec;
	const auto root = std::filesystem::absolute(projectRoot, ec);
	if (ec) { error = "Could not resolve the project directory."; return false; }
	registry_ = std::move(registry);
	root_ = root;
	models_.clear();
	return true;
}

std::shared_ptr<const QFE::ASSET::ModelAsset> QFE::ASSET::AssetManager::LoadLegacyModel(
	const std::string& name, const std::filesystem::path& modelDirectory, std::string& error) {
	error.clear();
	if (name.starts_with("asset-id/")) {
		const auto end = name.find('/', 9);
		if (end == std::string::npos) { error = "Invalid model asset key."; return {}; }
		return LoadModel(name.substr(9, end - 9), error);
	}
	const auto primitive = name == "box" ? std::string("Primitive/Box") : name;
	const auto key = "legacy:" + modelDirectory.generic_string() + ":" + name;
	const auto existing = models_.find(key);
	if (existing != models_.end()) return existing->second;
	auto result = std::make_shared<ModelAsset>();
	result->revision = 1;
	if (IsPrimitiveMeshName(primitive)) {
		auto vertices = CreatePrimitiveMesh(primitive);
		if (vertices.empty()) { error = "Could not create primitive model."; return {}; }
		auto data = std::make_shared<ModelData>();
		data->name = name;
		MeshData mesh(vertices.size(), vertices.size());
		mesh.vertices.GetInternalVector() = std::move(vertices);
		auto& indices = mesh.indices.GetInternalVector();
		indices.resize(mesh.vertices.size());
		for (uint32_t i = 0; i < indices.size(); ++i) indices[i] = i;
		data->meshes.push_back(std::move(mesh));
		result->ownedData = data;
		result->data = data.get();
	} else {
		std::string extension = std::filesystem::path(name).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		const auto source = modelDirectory / (name + (extension == ".glb" || extension == ".obj" ? "" : ".obj"));
		std::error_code ec;
		if (!std::filesystem::is_regular_file(source, ec) || ec) { error = "Model source was not found."; return {}; }
		const auto utf8 = source.generic_u8string();
		result->sourcePath.assign(reinterpret_cast<const char*>(utf8.data()), utf8.size());
		auto loader = std::make_shared<AssimpModelLoader>();
		loader->Initialize();
		ModelData loaded;
		if (!loader->LoadModel(result->sourcePath, loaded) || loaded.meshes.empty()) {
			error = "Could not load the model."; return {};
		}
		result->data = &loader->LoadModel(result->sourcePath);
		result->loader = std::move(loader);
	}
	models_[key] = result;
	return result;
}

std::shared_ptr<const QFE::ASSET::ModelAsset> QFE::ASSET::AssetManager::LoadModel(
	const std::string& id, std::string& error) {
	const auto found = models_.find(id);
	if (found != models_.end()) { error.clear(); return found->second; }
	return ReloadModel(id, error);
}

std::filesystem::path QFE::ASSET::AssetManager::ResolveSource(
	const std::string& id, const std::string& type, std::string& error) const {
	error.clear();
	const auto* entry = registry_.Find(id);
	if (!entry) { error = "Asset ID was not found."; return {}; }
	if (entry->type != type) { error = "Asset type does not match."; return {}; }
	const auto path = root_ / Utf8Path(entry->source);
	std::error_code ec;
	if (!QFE::PROJECT::IsWithin(path, root_) || !std::filesystem::is_regular_file(path, ec) || ec) {
		error = "Asset source was not found."; return {};
	}
	return path;
}

std::shared_ptr<const QFE::ASSET::ModelAsset> QFE::ASSET::AssetManager::ReloadModel(
	const std::string& id, std::string& error) {
	error.clear();
	const auto* entry = registry_.Find(id);
	if (!entry) { error = "Asset ID was not found."; return {}; }
	if (entry->type != "model") { error = "Asset is not a model."; return {}; }
	const auto path = root_ / std::filesystem::path(std::u8string(
		reinterpret_cast<const char8_t*>(entry->source.data()), entry->source.size()));
	std::error_code ec;
	if (!std::filesystem::is_regular_file(path, ec) || ec) { error = "Model source was not found."; return {}; }
	const auto utf8 = path.generic_u8string();
	auto result = std::make_shared<ModelAsset>();
	result->sourcePath.assign(reinterpret_cast<const char*>(utf8.data()), utf8.size());
	auto loader = std::make_shared<AssimpModelLoader>();
	loader->Initialize();
	ModelData loaded;
	if (!loader->LoadModel(result->sourcePath, loaded) || loaded.meshes.empty()) {
		error = "Could not load the model."; return {};
	}
	result->data = &loader->LoadModel(result->sourcePath);
	result->loader = std::move(loader);
	const auto previous = models_.find(id);
	if (previous != models_.end() && previous->second->revision == (std::numeric_limits<uint64_t>::max)()) {
		error = "Model revision overflow."; return {};
	}
	result->revision = previous == models_.end() ? 1 : previous->second->revision + 1;
	models_[id] = result;
	return result;
}

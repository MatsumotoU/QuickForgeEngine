#include "ModelLoader.h"
#include "PrimitiveFactoryFuncs.h"
#include <algorithm>
#include <cctype>
#include <limits>

namespace {
	std::filesystem::path Utf8Path(const std::string& text) {
		return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
	}
}

void QFE::ASSET::ModelLoader::Initialize() { models_.clear(); }

std::shared_ptr<const QFE::ASSET::ModelAsset> QFE::ASSET::ModelLoader::LoadModel(
	const std::string& name, const std::filesystem::path& modelDirectory, std::string& error) {
	error.clear();
	const auto primitive = name == "box" ? std::string("Primitive/Box") : name;
	const auto directoryText = modelDirectory.generic_u8string();
	const auto key = "legacy:" + std::string(reinterpret_cast<const char*>(directoryText.data()), directoryText.size()) + ":" + name;
	const auto existing = models_.find(key);
	if (existing != models_.end()) return existing->second;
	auto result = std::make_shared<ModelAsset>();
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
		std::string extension = Utf8Path(name).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		const auto source = modelDirectory / Utf8Path(name + (extension == ".glb" || extension == ".obj" ? "" : ".obj"));
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

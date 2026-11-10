#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/model/ModelLoader.h"
#include <fstream>
#include <iostream>
#include <iterator>

int wmain(int argc, wchar_t** argv) {
	if (argc != 4) return 2;
	const std::filesystem::path root(argv[1]), glb(argv[2]), obj(argv[3]);
	std::filesystem::create_directories(root / "assets");
	QFE::ASSET::AssetManager assets;
	std::string error;
	bool ok = true;
	auto check = [&](bool value, const char* message) { if (!value) { std::cerr << message << ": " << error << '\n'; ok = false; } };
	const auto bytes = [](const std::filesystem::path& file) {
		std::ifstream in(file, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	};
	check(assets.Open(root, error), "Opening project failed");
	check(assets.ImportRawFile(glb, error), "GLB copy failed");
	check(bytes(glb) == bytes(root / "assets/models" / glb.filename()), "GLB bytes changed");
	QFE::ASSET::ModelLoader models;
	models.Initialize();
	const auto filename = glb.filename().generic_u8string();
	const std::string modelName = "models/" + std::string(reinterpret_cast<const char*>(filename.data()), filename.size());
	const auto loaded = models.LoadModel(modelName, root / "assets", error);
	check(loaded && loaded->data && loaded->loader && !loaded->data->meshes.empty(), "Raw GLB did not load without a registry");
	check(loaded && !loaded->loader->GetGlbAnimationNames(loaded->sourcePath).empty(), "Raw GLB animation was not retained");
	check(loaded && models.LoadModel(modelName, root / "assets", error) == loaded && models.GetModels().size() == 1,
		"Model loader did not retain and reuse the model");
	check(!assets.ImportRawFile(glb, error), "Existing GLB was overwritten");
	check(bytes(glb) == bytes(root / "assets/models" / glb.filename()), "Existing GLB changed after duplicate import");
	check(assets.ImportRawFile(obj, error), "OBJ and dependencies copy failed");
	const auto copiedObj = root / "assets/models" / obj.stem() / obj.parent_path().filename() / obj.filename();
	check(bytes(obj) == bytes(copiedObj), "OBJ bytes changed");
	check(!assets.ImportRawFile(obj, error), "Existing OBJ was overwritten");
	const auto input = root / "sample.wav";
	{ std::ofstream out(input, std::ios::binary); out << "audio bytes"; }
	check(assets.ImportRawFile(input, error) && bytes(root / "assets/audio/sample.wav") == "audio bytes", "Audio copy failed");
	const auto broken = root / "missing.obj";
	{ std::ofstream out(broken); out << "mtllib missing.mtl\n"; }
	check(!assets.ImportRawFile(broken, error) && !std::filesystem::exists(root / "assets/models/missing" / root.filename() / "missing.obj"),
		"Missing OBJ dependency left a copied model");
	check(!assets.ListProjectFiles(error).empty() && error.empty(), "Listing project files failed");
	check(!std::filesystem::exists(root / "asset-registry.json"), "Listing wrote a registry");
	return ok ? 0 : 1;
}

#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/game-data/GamePak.h"
#include <fstream>
#include <iostream>

int wmain(int argc, wchar_t** argv) {
	if (argc != 4) return 2;
	const std::filesystem::path root(argv[1]);
	std::filesystem::create_directories(root / "assets/models");
	const auto modelPath = root / "assets/models/test.glb";
	std::filesystem::copy_file(argv[2], modelPath);
	QFE::ASSET::AssetManager manager;
	std::string error;
	bool ok = true;
	const auto check = [&](bool value, const char* message) { if (!value) { std::cerr << message << '\n'; ok = false; } };
	check(manager.Open(root, error), "Open failed");
	const auto importedId = manager.ImportGlb(std::filesystem::path(argv[2]), error);
	check(!importedId.empty() && manager.Registry().Find(importedId), "GLB import failed");
	const auto imported = manager.LoadModel(importedId, error);
	check(imported && imported->data && !imported->data->meshes.empty(), "Imported GLB did not load");
	const auto objId = manager.ImportObj(std::filesystem::path(argv[3]), error);
	check(!objId.empty() && manager.Registry().Find(objId), "OBJ import failed");
	const auto obj = manager.LoadModel(objId, error);
	check(obj && obj->data && !obj->data->meshes.empty() &&
		std::filesystem::is_regular_file(obj->data->meshes[0].material.textureName), "OBJ material texture was not imported");
	check(manager.ReplaceModel(objId, std::filesystem::path(argv[3]), error), "OBJ replacement failed");
	const auto replacedObj = manager.LoadModel(objId, error);
	check(replacedObj && obj && replacedObj->revision == obj->revision + 1 &&
		std::filesystem::is_regular_file(replacedObj->data->meshes[0].material.textureName), "OBJ replacement lost its material texture");
	const auto brokenObj = root / "broken.obj";
	{ std::ofstream file(brokenObj); file << "mtllib missing.mtl\n"; }
	check(manager.ImportObj(brokenObj, error).empty(), "OBJ with missing material was imported");
	check(!manager.ReplaceModel(objId, brokenObj, error) && manager.LoadModel(objId, error) == replacedObj,
		"Failed OBJ replacement changed the model");
	{ std::ofstream material(root / "missing.mtl"); material << "newmtl m\nmap_Kd missing.png\n"; }
	check(manager.ImportObj(brokenObj, error).empty(), "OBJ with missing texture was imported");
	check(!manager.ReplaceModel(objId, brokenObj, error) && manager.LoadModel(objId, error) == replacedObj,
		"OBJ with missing texture replaced the model");
	check(manager.ReplaceModel(importedId, std::filesystem::path(argv[2]), error), "GLB replacement failed");
	const auto replaced = manager.LoadModel(importedId, error);
	check(replaced && imported && replaced->revision == imported->revision + 1 && replaced->sourcePath != imported->sourcePath,
		"Replacement did not preserve ID and update the model revision");
	check(imported && imported->data && !imported->data->meshes.empty(), "Replacement invalidated the previous model");
	const auto legacy = manager.LoadLegacyModel("test.glb", root / "assets/models", error);
	check(legacy && legacy->data && legacy->loader && !legacy->data->meshes.empty(), "Legacy GLB load failed");
	check(legacy && !legacy->loader->GetGlbAnimationNames(legacy->sourcePath).empty(), "Legacy GLB animation was lost");
	std::vector<MeshVertexData> sampled;
	check(legacy && legacy->loader->SampleGlbAnimation(legacy->sourcePath, "", 0.1, sampled) &&
		legacy->data && sampled.size() == legacy->data->meshes[0].vertices.size(), "Legacy GLB animation sampling failed");
	const auto primitive = manager.LoadLegacyModel("box", root / "assets/models", error);
	check(primitive && primitive->data && !primitive->data->meshes.empty(), "Primitive load failed");
	check(manager.Registry().Register({"model-1", "model", "test", "assets/models/test.glb", {}, {}}, error), "Register failed");
	{ std::ofstream material(root / "assets/models/test.mtl"); material << "newmtl test"; }
	{ std::ofstream image(root / "assets/models/icon.png"); image << "image"; }
	check(manager.RegisterProjectFiles(error), "Legacy project registration failed");
	check(manager.Registry().List().size() == 4 && manager.Registry().Find("model-1"), "Existing ID changed or MTL was registered");
	check(manager.RegisterProjectFiles(error) && manager.Registry().List().size() == 4, "Repeated registration created duplicates");
	std::string imageId;
	for (const auto& asset : manager.Registry().List()) {
		if (asset.source == "assets/models/icon.png") {
			imageId = asset.id;
			check(manager.ResolveSource(asset.id, "image", error) == root / "assets/models/icon.png", "Image ID did not resolve");
			check(manager.ResolveSource(asset.id, "model", error).empty(), "Wrong asset type was accepted");
		}
	}
	QFE::ASSET::AssetManager reopened;
	check(reopened.Open(root, error) && reopened.Registry().Find("model-1") && reopened.Registry().List().size() == 4, "Registration was not persisted");
	check(reopened.LoadModel(importedId, error) != nullptr, "Imported GLB did not survive reopening");
	std::filesystem::create_directories(root / "assets/scenes");
	{ std::ofstream project(root / "project.json"); project << R"({"start_scene":"start.scene"})"; }
	{ std::ofstream scene(root / "assets/scenes/start.scene"); scene << "{}"; }
	const auto package = root / "game.qfepak";
	const auto unpacked = root / "unpacked";
	std::filesystem::create_directory(unpacked);
	check(QFE::PAK::Write(root, package, error) && QFE::PAK::Extract(package, unpacked, error), "Package round-trip failed");
	QFE::ASSET::AssetManager packaged;
	check(packaged.Open(unpacked, error) && packaged.LoadModel(importedId, error) != nullptr, "Package did not preserve imported model ID and source");
	check(packaged.LoadModel(objId, error) != nullptr, "Package did not preserve imported OBJ");
	check(!imageId.empty() && !packaged.ResolveSource(imageId, "image", error).empty(), "Package did not preserve image asset ID");
	const auto first = manager.LoadModel("model-1", error);
	check(first && first->data && !first->data->meshes.empty() && first->revision == 1, "Model load failed");
	check(manager.LoadLegacyModel("asset-id/model-1/1/test.glb", root / "assets", error) == first, "Registered model key did not resolve");
	check(manager.LoadModel("model-1", error) == first, "Model was not cached");
	const auto second = manager.ReloadModel("model-1", error);
	check(second && second != first && second->revision == 2, "Reload did not create a new revision");
	check(first && first->data && !first->data->meshes.empty(), "Reload invalidated a previous reference");
	{ std::ofstream bad(modelPath, std::ios::binary | std::ios::trunc); bad << "invalid model"; }
	check(manager.ImportGlb(modelPath, error).empty(), "Invalid model import succeeded");
	check(!manager.ReplaceModel(importedId, modelPath, error) && manager.LoadModel(importedId, error) == replaced,
		"Failed replacement changed the model");
	check(!manager.ReloadModel("model-1", error), "Invalid source reload succeeded");
	check(manager.LoadModel("model-1", error) == second, "Invalid source replaced cached model");
	std::filesystem::remove(modelPath);
	check(!manager.ReloadModel("model-1", error), "Missing source reload succeeded");
	check(manager.LoadModel("model-1", error) == second, "Failed reload replaced cached model");
	std::filesystem::copy_file(argv[2], modelPath);
	check(manager.RegisterProjectFiles(error), "Registration before replacement failed");
	const auto assetCountBeforeReplacement = manager.Registry().List().size();
	check(manager.ReplaceModel("model-1", std::filesystem::path(argv[2]), error), "Legacy model replacement failed");
	check(manager.RegisterProjectFiles(error) && manager.Registry().List().size() == assetCountBeforeReplacement,
		"Replacement caused the old source to be registered as a duplicate");
	return ok ? 0 : 1;
}

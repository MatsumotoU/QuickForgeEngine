#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/game-data/GamePak.h"
#include "framework/application/RetiredModelResources.h"
#include "framework/application/TextureReferencePolicy.h"
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
	check(QFE::FRAMEWORK::IsDdsTexture("sky/sky.DdS") && !QFE::FRAMEWORK::IsDdsTexture("images/fire.png") &&
		!QFE::FRAMEWORK::IsDdsTexture("model.glb#*0"), "DDS texture classification failed");
	const auto utf8Path = [](const std::string& value) {
		return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size()));
	};
	const std::unordered_map<std::string, uint64_t> revisions{{"model-1", 2}, {"model-2", 4}};
	check(QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/1/models/test.glb", revisions), "Previous model revision was not retired");
	check(!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/2/models/test.glb", revisions) &&
		!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/3/models/test.glb", revisions) &&
		!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/unknown/1/models/test.glb", revisions) &&
		!QFE::FRAMEWORK::IsRetiredModelResource("models/test.glb", revisions), "Current, future, unknown or legacy model was retired");
	check(!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/0/x", revisions) &&
		!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/1junk/x", revisions) &&
		!QFE::FRAMEWORK::IsRetiredModelResource("asset-id/model-1/1/", revisions), "Invalid model cache key was retired");
	std::unordered_map<uint32_t, uint32_t> animatedBuffers{{1, 100}, {2, 200}, {3, 300}};
	const std::unordered_set<uint32_t> activeBuffers{1, 3};
	const auto retiredBuffers = QFE::FRAMEWORK::CollectUnusedAnimatedBuffers(animatedBuffers, activeBuffers);
	check(retiredBuffers.size() == 1 && retiredBuffers[0] == std::pair<uint32_t, uint32_t>{2, 200} &&
		animatedBuffers.size() == 2 && animatedBuffers.contains(1) && animatedBuffers.contains(3),
		"Unused animated buffer collection removed an active resource or kept an unused one");
	check(QFE::FRAMEWORK::CollectUnusedAnimatedBuffers(animatedBuffers, activeBuffers).empty(),
		"Animated buffer was queued more than once");
	check(manager.Open(root, error), "Open failed");
	const auto importedId = manager.ImportGlb(std::filesystem::path(argv[2]), error);
	check(!importedId.empty() && manager.Registry().Find(importedId), "GLB import failed");
	const auto imported = manager.LoadModel(importedId, error);
	check(imported && imported->data && !imported->data->meshes.empty(), "Imported GLB did not load");
	check(imported && imported->embeddedImageIds.size() == 1 &&
		imported->data->meshes[0].material.embeddedTextureIndex == 0, "GLB embedded image was not registered or mapped");
	const std::string embeddedImageId = imported && !imported->embeddedImageIds.empty() ? imported->embeddedImageIds[0] : "";
	check(!embeddedImageId.empty() && !manager.ResolveSource(embeddedImageId, "image", error).empty(), "Embedded image ID did not resolve");
	const auto objId = manager.ImportObj(std::filesystem::path(argv[3]), error);
	check(!objId.empty() && manager.Registry().Find(objId), "OBJ import failed");
	const auto obj = manager.LoadModel(objId, error);
	check(obj && obj->data && !obj->data->meshes.empty() &&
		std::filesystem::is_regular_file(utf8Path(obj->data->meshes[0].material.textureName)), "OBJ material texture was not imported");
	check(manager.ReplaceModel(objId, std::filesystem::path(argv[3]), error), "OBJ replacement failed");
	const auto replacedObj = manager.LoadModel(objId, error);
	check(replacedObj && obj && replacedObj->revision == obj->revision + 1 &&
		std::filesystem::is_regular_file(utf8Path(replacedObj->data->meshes[0].material.textureName)), "OBJ replacement lost its material texture");
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
	check(replaced && imported && replaced->embeddedImageIds == imported->embeddedImageIds, "GLB replacement changed embedded image IDs");
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
	const size_t registeredCount = 4 + (imported ? imported->embeddedImageIds.size() : 0);
	check(manager.Registry().List().size() == registeredCount && manager.Registry().Find("model-1"), "Existing ID changed or MTL was registered");
	check(manager.RegisterProjectFiles(error) && manager.Registry().List().size() == registeredCount, "Repeated registration created duplicates");
	check(manager.FindLegacyAssetId("models/test.glb", "model") == "model-1" &&
		manager.FindLegacyAssetId("resources/models/test.glb", "model") == "model-1" &&
		manager.FindLegacyAssetId("box", "model").empty() && manager.FindLegacyAssetId("../outside.glb", "model").empty(),
		"Legacy model path did not resolve safely");
	std::string imageId;
	for (const auto& asset : manager.Registry().List()) {
		if (asset.source == "assets/models/icon.png") {
			imageId = asset.id;
			check(manager.ResolveSource(asset.id, "image", error) == root / "assets/models/icon.png", "Image ID did not resolve");
			check(manager.ResolveSource(asset.id, "model", error).empty(), "Wrong asset type was accepted");
		}
	}
	QFE::ASSET::AssetManager reopened;
	check(reopened.Open(root, error) && reopened.Registry().Find("model-1") && reopened.Registry().List().size() == registeredCount, "Registration was not persisted");
	check(reopened.Registry().Register({"duplicate", "model", "duplicate", "assets/models/test.glb", {}, {}}, error) &&
		reopened.FindLegacyAssetId("models/test.glb", "model").empty(), "Ambiguous legacy path resolved to an arbitrary ID");
	check(reopened.LoadModel(importedId, error) != nullptr, "Imported GLB did not survive reopening");
	const auto imagePath = std::filesystem::path(argv[3]).parent_path().parent_path() / "images/ship.png";
	const auto optionObj = root / "options.obj";
	{ std::ofstream file(optionObj); file << "mtllib options.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nvt 1 0\nvt 0 1\nusemtl m\nf 1/1 2/2 3/3\n"; }
	std::filesystem::copy_file(imagePath, root / "option texture.png");
	{ std::ofstream file(root / "options.mtl"); file << "newmtl m\nmap_Kd -clamp on -o -1 0 0 -s 1 1 1 option texture.png\n"; }
	const auto optionModelId = manager.ImportObj(optionObj, error);
	check(!optionModelId.empty() && manager.LoadModel(optionModelId, error), "OBJ texture options were not imported");
	{ std::ofstream file(root / "options.mtl"); file << "newmtl m\nmap_Kd -unknown 1 option texture.png\n"; }
	check(manager.ImportObj(optionObj, error).empty(), "Unsupported MTL option was accepted");
	{ std::ofstream file(root / "first material.mtl"); file << "newmtl m\nKd 1 1 1\nmap_Kd option texture.png\n"; }
	{ std::ofstream file(root / "second.mtl"); file << "newmtl m2\nKd 1 0 0\n"; }
	{ std::ofstream file(optionObj); file << "mtllib \"first material.mtl\" second.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nusemtl m\nf 1 2 3\n"; }
	const auto multiMaterialId = manager.ImportObj(optionObj, error);
	check(!multiMaterialId.empty(), "OBJ with multiple material libraries failed to import");
	if (!multiMaterialId.empty()) {
		const auto copiedDir = manager.ResolveSource(multiMaterialId, "model", error).parent_path();
		check(std::filesystem::is_regular_file(copiedDir / "material-0.mtl") &&
			std::filesystem::is_regular_file(copiedDir / "material-1.mtl"), "OBJ material libraries were not copied");
		const auto multiModel = manager.LoadModel(multiMaterialId, error);
		check(multiModel && multiModel->data && !multiModel->data->meshes.empty() &&
			std::filesystem::is_regular_file(utf8Path(multiModel->data->meshes[0].material.textureName)),
			"OBJ with multiple material libraries lost its texture");
	}
	const auto differentGlb = std::filesystem::path(argv[2]).parent_path() / "monkey.glb";
	check(!manager.ReplaceModel(importedId, differentGlb, error) &&
		error == "Embedded images changed; their asset IDs cannot be matched automatically." && manager.LoadModel(importedId, error) == replaced,
		"Changed GLB embedded images were silently remapped");
	check(manager.ReplaceImage(embeddedImageId, imagePath, error) &&
		manager.LoadModel(importedId, error)->embeddedImageIds[0] == embeddedImageId,
		"Replacing an embedded image lost its model mapping");
	const auto importedImageId = manager.ImportImage(imagePath, error);
	check(!importedImageId.empty(), "Image import failed");
	const auto firstImage = manager.ResolveSource(importedImageId, "image", error);
	check(!firstImage.empty() && std::filesystem::is_regular_file(firstImage), "Imported image did not resolve");
	check(manager.ReplaceImage(importedImageId, imagePath, error), "Image replacement failed");
	const auto replacementImage = manager.ResolveSource(importedImageId, "image", error);
	check(!replacementImage.empty() && replacementImage != firstImage && std::filesystem::is_regular_file(firstImage),
		"Image replacement did not preserve ID and old source");
	const auto badImage = root / "bad.png";
	{ std::ofstream file(badImage); file << "not an image"; }
	check(manager.ImportImage(badImage, error).empty(), "Invalid image import succeeded");
	check(!manager.ReplaceImage(importedImageId, badImage, error) && manager.ResolveSource(importedImageId, "image", error) == replacementImage,
		"Invalid image replacement changed the source");
	{ std::ofstream file(root / "asset-registry.json.qfe-writing"); file << "blocked"; }
	const auto countBeforeFailedImport = manager.Registry().List().size();
	const auto imageFileCount = std::distance(std::filesystem::directory_iterator(root / "sources/images"), std::filesystem::directory_iterator{});
	check(manager.ImportGlb(std::filesystem::path(argv[2]), error).empty() && manager.Registry().List().size() == countBeforeFailedImport &&
		std::distance(std::filesystem::directory_iterator(root / "sources/images"), std::filesystem::directory_iterator{}) == imageFileCount,
		"Failed GLB import left registered or copied embedded images");
	check(manager.ImportImage(imagePath, error).empty() && manager.Registry().List().size() == countBeforeFailedImport,
		"Failed registry save registered an image");
	check(!manager.ReplaceImage(importedImageId, imagePath, error) && manager.ResolveSource(importedImageId, "image", error) == replacementImage,
		"Failed registry save changed the image source");
	std::filesystem::remove(root / "asset-registry.json.qfe-writing");
	QFE::ASSET::AssetManager imageReopened;
	check(imageReopened.Open(root, error) && imageReopened.ResolveSource(importedImageId, "image", error) == replacementImage,
		"Image replacement did not survive reopening");
	const auto bitmapPath = root / "replacement.bmp";
	const unsigned char bitmap[] = {
		'B', 'M', 58, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
		40, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 24, 0,
		0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0
	};
	{ std::ofstream file(bitmapPath, std::ios::binary); file.write(reinterpret_cast<const char*>(bitmap), sizeof(bitmap)); }
	check(manager.ReplaceImage(importedImageId, bitmapPath, error) &&
		manager.ResolveSource(importedImageId, "image", error).extension() == ".bmp" &&
		manager.Registry().Find(importedImageId)->name.ends_with(".bmp"), "Image replacement across formats failed");
	const auto countBeforeLegacyImageReplacement = manager.Registry().List().size();
	check(!imageId.empty() && manager.ReplaceImage(imageId, imagePath, error), "Legacy image replacement failed");
	check(manager.FindLegacyAssetId("models/icon.png", "image") == imageId, "Old image path did not resolve after replacement");
	check(manager.RegisterProjectFiles(error) && manager.Registry().List().size() == countBeforeLegacyImageReplacement,
		"Legacy image replacement registered the old source again");
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
	check(!packaged.ResolveSource(importedImageId, "image", error).empty(), "Package did not preserve imported image ID");
	const auto packagedModel = packaged.LoadModel(importedId, error);
	check(!packaged.ResolveSource(embeddedImageId, "image", error).empty() && packagedModel &&
		packagedModel->embeddedImageIds.size() == 1 && packagedModel->embeddedImageIds[0] == embeddedImageId,
		"Package did not preserve embedded image mapping");
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
	check(manager.FindLegacyAssetId("models/test.glb", "model") == "model-1", "Old model path did not resolve after replacement");
	check(manager.RegisterProjectFiles(error) && manager.Registry().List().size() == assetCountBeforeReplacement + 1,
		"Replacement caused the old source to be registered as a duplicate");
	return ok ? 0 : 1;
}

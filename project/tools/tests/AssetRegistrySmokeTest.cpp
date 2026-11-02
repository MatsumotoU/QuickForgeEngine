#include "assetfactory/game-data/AssetRegistry.h"
#include <fstream>
#include <iostream>

int wmain(int argc, wchar_t** argv) {
	if (argc != 2) return 2;
	const std::filesystem::path root(argv[1]);
	std::filesystem::create_directories(root / "assets");
	QFE::ASSET::AssetRegistry registry;
	std::string error;
	bool ok = true;
	const auto check = [&](bool value, const char* message) { if (!value) { std::cerr << message << '\n'; ok = false; } };
	check(registry.Open(root, error), "Failed to open empty project");
	const QFE::ASSET::AssetMetadata model{"asset-1", "model", "ship", "assets/models/ship.glb", {"texture-1"}, {}};
	check(registry.Register(model, error), "Registration failed");
	check(!registry.Register(model, error), "Duplicate ID accepted");
	check(!registry.Register({"bad", "model", "bad", "../outside.glb", {}, {}}, error), "Traversal accepted");
	check(!registry.Register({"bad/id", "model", "bad", "assets/bad.glb", {}, {}}, error), "Invalid ID accepted");
	QFE::ASSET::AssetMetadata replacement = model;
	replacement.source = "sources/models/ship.glb";
	replacement.previousSources.push_back(model.source);
	check(registry.Update(replacement, error), "Update failed");
	check(registry.Save(error), "Save failed");
	QFE::ASSET::AssetRegistry restored;
	check(restored.Open(root, error), "Reload failed");
	const auto* entry = restored.Find("asset-1");
	check(entry && entry->source == replacement.source && entry->dependencies == model.dependencies &&
		entry->previousSources == replacement.previousSources, "Registry did not round-trip");
	{ std::ofstream out(root / "asset-registry.json"); out << "{ invalid"; }
	check(!restored.Open(root, error), "Malformed registry accepted");
	check(restored.Find("asset-1") != nullptr, "Failed open destroyed existing state");
	return ok ? 0 : 1;
}

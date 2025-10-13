#pragma once
#include "../IEditorUI.h"
#include <unordered_map>
#include <functional>

class AssetManager;

enum class LoadSpace {
	Memory,
	File
};

enum class ViewHierarchy {
	Root,
	Images,
	Models,
	Materials,
	Shaders,
	Audio,
	Others
};

class AssetsView : public IEditorUI {
public:
	AssetsView();
	~AssetsView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

	void RootView();
	void ImagesView();
	void ModelsView();
	void MaterialsView();
	void ShadersView();
	void AudioView();
	void OthersView();

	void FilesView();

private:
	AssetManager* assetManager;
	ViewHierarchy currentHierarchy;
	std::unordered_map<ViewHierarchy, std::function<void()>> drawFunctions;
	std::unordered_map<ViewHierarchy, std::string> hierarchyNames;

	LoadSpace loadSpace_;

	uint32_t fileGH_;
	uint32_t arrowGH_;

};
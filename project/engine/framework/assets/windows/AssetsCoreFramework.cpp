#include "framework/assets/AssetsCoreFramework.h"
#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include "assetfactory/model/ModelLoader.h"
#include "assetfactory/model/PrimitiveFactoryFuncs.h"

namespace QFE::FRAMEWORK::ASSETS {
	// エラー内容と一覧順は AssetManager の結果をそのまま返す。
	bool OpenProject(QFE::ASSET::AssetManager& assets, const std::filesystem::path& root, std::string& error) {
		return assets.Open(root, error);
	}
	bool ImportFile(QFE::ASSET::AssetManager& assets, const std::filesystem::path& source, std::string& error) {
		return assets.ImportRawFile(source, error);
	}
	std::vector<std::string> ListProjectFiles(const QFE::ASSET::AssetManager& assets, std::string& error) {
		return assets.ListProjectFiles(error);
	}
	void InitializeModelLoader(QFE::ASSET::ModelLoader& models) { models.Initialize(); }
	std::vector<std::string> GetPrimitiveMeshNames() { return QFE::ASSET::GetPrimitiveMeshNames(); }
	std::vector<std::string> GetModelAnimationNames(QFE::ASSET::ModelLoader& models,
		const std::string& name, const std::filesystem::path& directory, std::string& error) {
		// ローダーの寿命はモデル資産が保持する。組み込みモデルなど GLB がない場合は空を返す。
		const auto asset = models.LoadModel(name, directory, error);
		return asset && asset->loader ? asset->loader->GetGlbAnimationNames(asset->sourcePath) : std::vector<std::string>{};
	}
	void SetProjectRoot(const std::filesystem::path& root) { QFE::PROJECT::SetRoot(root); }
	std::filesystem::path GetProjectRoot() { return QFE::PROJECT::Root(); }
	std::filesystem::path GetProjectAssets() { return QFE::PROJECT::Assets(); }
	std::filesystem::path ResolveProjectAsset(const std::filesystem::path& path) { return QFE::PROJECT::Resolve(path); }
	bool IsPathWithin(const std::filesystem::path& path, const std::filesystem::path& directory) {
		return QFE::PROJECT::IsWithin(path, directory);
	}
}

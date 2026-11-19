#pragma once
#include <filesystem>
#include <string>

namespace QFE::ASSET { class AssetManager; class ModelLoader; }
namespace QFE::SCENE { class SceneManager; }

namespace QFE::FRAMEWORK {
	class LuaScriptSystem;
	/// @brief 指定のパスにプロジェクトのアセットディレクトリを作成する。
	/// @param projectPath プロジェクトのルート。
	/// @param assetsPath アセットのルート。相対パスの場合は projectPath を基準にする。
	void CreateProjectAssets(const std::filesystem::path& projectPath, const std::filesystem::path& assetsPath);

	/// @brief プロジェクト設定と開始シーンを検証し、必要ならデフォルトシーンを作成する。
	/// @return 読み込む開始シーンのパス。失敗時は例外を送出する。
	std::filesystem::path InitializeProjectStartupScene(const std::filesystem::path& projectPath,
		const SCENE::SceneManager& sceneManager);

	/// @brief プロジェクトのアセットとスクリプトの参照先を初期化する。
	bool InitializeProjectAssets(const std::filesystem::path& projectPath,
		ASSET::AssetManager& assets, ASSET::ModelLoader& models, LuaScriptSystem& scripts,
		std::string& assetDir, std::string& modelDir, std::string& scriptDir, std::string& error);
}

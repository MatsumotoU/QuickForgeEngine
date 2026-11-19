#include "WindowsAssetsFramework.h"
#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include "assetfactory/model/ModelLoader.h"
#include "core/string/MyString.h"
#include "framework/application/LuaScriptSystem.h"
#include "scene/SceneManager.h"
#include <fstream>
#include <stdexcept>

void QFE::FRAMEWORK::CreateProjectAssets(const std::filesystem::path& projectPath, const std::filesystem::path& assetsPath)
{
	const auto root = projectPath / assetsPath;
	for (const auto* dir : { L"scenes", L"entities", L"models", L"images", L"audio", L"scripts", L"shaders", L"animations" }) {
		std::filesystem::create_directories(root / dir);
	}
}

bool QFE::FRAMEWORK::InitializeProjectAssets(const std::filesystem::path& projectPath,
	ASSET::AssetManager& assets, ASSET::ModelLoader& models, LuaScriptSystem& scripts,
	std::string& assetDir, std::string& modelDir, std::string& scriptDir, std::string& error)
{
	if (!assets.Open(projectPath, error)) return false;
	models.Initialize();
	const auto projectAssets = projectPath / "assets";
	assetDir = QFE::ConvertString(projectAssets.wstring()) + "/";
	modelDir = assetDir;
	scriptDir = QFE::ConvertString((projectAssets / "scripts").wstring());
	scripts.Reset();
	scripts.Initialize(std::filesystem::path(QFE::ConvertString(scriptDir)));
	return true;
}

std::filesystem::path QFE::FRAMEWORK::InitializeProjectStartupScene(
	const std::filesystem::path& projectPath, const SCENE::SceneManager& sceneManager)
{
	const auto projectFile = projectPath / L"project.json";
	const auto scenesDirectory = projectPath / L"assets" / L"scenes";
	std::filesystem::path startupScene;
	nlohmann::json project;
	// 既存の設定を読み込み、まだなければプロジェクトの初期設定を用意する。
	const bool projectExists = std::filesystem::exists(projectFile);
	if (projectExists) {
		std::ifstream input(projectFile, std::ios::binary);
		project = nlohmann::json::parse(input);
		if (!project.is_object()) throw std::runtime_error("Invalid project.json");
	} else {
		project = { {"name", QFE::ConvertString(projectPath.filename().wstring())}, {"version", 1} };
	}
	const auto configured = project.find("start_scene");
	bool updateProject = false;
	if (configured != project.end() && !configured->is_string()) throw std::runtime_error("Invalid start_scene");
	const std::string sceneName = configured == project.end() ? std::string{} : configured->get<std::string>();
	if (!sceneName.empty()) {
		// start_scene は assets/scenes からの相対パスとして解決する。
		const std::filesystem::path relativeScene(QFE::ConvertString(sceneName));
		startupScene = scenesDirectory / relativeScene;
		if (relativeScene.is_absolute() || !QFE::PROJECT::IsWithin(startupScene, scenesDirectory)) {
			throw std::runtime_error("start_scene must be inside assets/scenes");
		}
	}
	// 指定がない、またはファイルがない場合はデフォルトシーンを使う。
	if (startupScene.empty() || !std::filesystem::exists(startupScene)) {
		startupScene = scenesDirectory / L"DefaultScene.scene";
		if (!QFE::PROJECT::IsWithin(startupScene, scenesDirectory)) throw std::runtime_error("Invalid DefaultScene path");
		if (!std::filesystem::exists(startupScene)) {
			// 初期カメラとライトを含む現在のシーンを保存する。
			std::ofstream scene(startupScene, std::ios::binary);
			scene << sceneManager.CreateSnapshot().sceneJson.dump(4) << '\n';
			scene.close();
			if (!scene) throw std::runtime_error("Could not create DefaultScene");
		}
		project["start_scene"] = "DefaultScene.scene";
		updateProject = true;
	}
	// プロジェクト設定を書き換える前に、開始シーンの基本形式を確認する。
	std::ifstream sceneFile(startupScene, std::ios::binary);
	const auto scene = nlohmann::json::parse(sceneFile);
	if (!scene.is_object() || !scene.contains("entities") || !scene["entities"].is_array()) {
		throw std::runtime_error("Invalid startup scene");
	}
	// デフォルトシーンを使う場合は、次回以降も同じシーンを開けるようにする。
	if (updateProject) {
		std::ofstream file(projectFile, std::ios::binary | std::ios::trunc);
		file << project.dump(4) << '\n';
		file.close();
		if (!file) throw std::runtime_error("Could not save project.json");
	}
	return startupScene;
}

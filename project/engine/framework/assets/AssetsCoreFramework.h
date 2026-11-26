#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "math/vector/Vector3.h"

namespace QFE::ASSET { class AssetManager; class ModelLoader; }

namespace QFE::FRAMEWORK::ASSETS {
	// アセットの所有者は従来どおりモジュールの型を使う。
	// 操作はこの窓口に集め、アプリケーションが読み込み実装に依存しないようにする。
	bool OpenProject(QFE::ASSET::AssetManager& assets, const std::filesystem::path& root, std::string& error);
	bool ImportFile(QFE::ASSET::AssetManager& assets, const std::filesystem::path& source, std::string& error);
	std::vector<std::string> ListProjectFiles(const QFE::ASSET::AssetManager& assets, std::string& error);
	void InitializeModelLoader(QFE::ASSET::ModelLoader& models);
	// エディターはモデルローダーに直接依存せず、描画専用の三角形を取得する。
	bool GetModelPreviewTriangles(QFE::ASSET::ModelLoader& models, const std::string& name,
		std::vector<QFE::MATH::Vector3>& triangles, std::string& error);

	/// 組み込みモデルの名前と、モデル内の GLB アニメーション名を取得する。
	std::vector<std::string> GetPrimitiveMeshNames();
	std::vector<std::string> GetModelAnimationNames(QFE::ASSET::ModelLoader& models,
		const std::string& name, const std::filesystem::path& directory, std::string& error);

	// ルートは従来と同じプロセス内の保存先を使う。
	// 未設定時の resources/ へのフォールバックも変更しない。
	void SetProjectRoot(const std::filesystem::path& root);
	std::filesystem::path GetProjectRoot();
	std::filesystem::path GetProjectAssets();
	std::filesystem::path ResolveProjectAsset(const std::filesystem::path& path);
	/// シンボリックリンクを解決して判定する。基準ディレクトリ自身は含まない。
	bool IsPathWithin(const std::filesystem::path& path, const std::filesystem::path& directory);
}

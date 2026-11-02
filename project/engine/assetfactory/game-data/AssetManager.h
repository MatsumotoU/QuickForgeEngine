#pragma once
#include "AssetRegistry.h"
#include "assetfactory/model/AssimpModelLoader.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace QFE::ASSET {
	/// @brief モデル本体と、そのGLBアニメーションを保持する読み込み結果。
	struct ModelAsset {
		const ModelData* data = nullptr; ///< loaderが所有するモデル。loaderより長く使わない。
		std::shared_ptr<const ModelData> ownedData; ///< 組み込みモデル用データ。
		std::shared_ptr<const AssimpModelLoader> loader; ///< GLBのボーン・クリップデータの所有元。
		std::string sourcePath;
		uint64_t revision = 0;
	};

	/// @brief プロジェクトの登録情報とCPU側アセットを管理する。
	class AssetManager final {
	public:
		/// @brief プロジェクトを開き、登録内容を読み込む。
		bool Open(const std::filesystem::path& projectRoot, std::string& error);
		/// @brief 登録済みモデルを読み込む。失敗時はnullptrとエラーを返す。
		std::shared_ptr<const ModelAsset> LoadModel(const std::string& id, std::string& error);
		/// @brief モデルを再読み込みする。失敗時は既存データを保持する。
		std::shared_ptr<const ModelAsset> ReloadModel(const std::string& id, std::string& error);
		/// @brief 旧シーンのモデル名からCPUデータを取得する。
		std::shared_ptr<const ModelAsset> LoadLegacyModel(const std::string& name, const std::filesystem::path& modelDirectory, std::string& error);
		/// @brief 旧プロジェクトのassets以下を登録する。既存IDは維持する。
		bool RegisterProjectFiles(std::string& error);
		/// @brief GLBモデルを検証してプロジェクトへ取り込み、アセットIDを返す。
		std::string ImportGlb(const std::filesystem::path& source, std::string& error);
		/// @brief OBJと参照するMTL・画像を検証してプロジェクトへ取り込む。
		std::string ImportObj(const std::filesystem::path& source, std::string& error);
		/// @brief 登録済みモデルを同じIDのまま置き換える。失敗時は既存データを保持する。
		bool ReplaceModel(const std::string& id, const std::filesystem::path& source, std::string& error);
		/// @brief 登録IDと種類からプロジェクト内の元ファイルを解決する。
		std::filesystem::path ResolveSource(const std::string& id, const std::string& type, std::string& error) const;
		/// @brief 登録情報を取得する。
		AssetRegistry& Registry() { return registry_; }
		/// @brief 読み取り専用の登録情報を取得する。
		const AssetRegistry& Registry() const { return registry_; }
	private:
		std::filesystem::path root_;
		AssetRegistry registry_;
		std::unordered_map<std::string, std::shared_ptr<const ModelAsset>> models_;
	};
}

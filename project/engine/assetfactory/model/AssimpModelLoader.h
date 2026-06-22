#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "ModelData.h"

namespace QFE::ASSET {
	/// @brief Assimpを使用してモデルデータを読み込むクラス,基本はモデルをキャッシュする
	class AssimpModelLoader {
	public:
		/// @brief キャッシュを初期化します
		void Initialize();
		/// @brief 指定されたファイルパスからモデルデータを読み込む関数,キャッシュに存在する場合はキャッシュから返す
		ModelData& LoadModel(const std::string& filePath);
		/// @brief 指定されたファイルパスからモデルデータを強制的に読み込む関数,キャッシュを無視して新たに読み込む,キャッシュに存在する場合は上書きする
		ModelData& ForceLoadModel(const std::string& filePath);
		/// @brief キャッシュにモデルデータが存在するかどうかを確認する関数
		bool IsModelCached(const std::string& filePath) const;

	private:
		/// @brief 指定されたファイルパスからモデルデータを読み込む関数
		void LoadModelData(const std::string& filePath, ModelData& modelData);

		std::unordered_map<std::string, ModelData> modelCache; // モデルデータのキャッシュ
		ModelData invalidModelData; // 無効なモデルデータを返すためのメンバ変数
	};

}

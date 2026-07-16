#pragma once
#include <string>
#include <vector>


#include "ModelData.h"
#include "SkinningModelData.h"

namespace QFE::ASSET {
	/// @brief Assimpを使用してモデルデータを読み込むクラス,基本はモデルをキャッシュする
	class AssimpModelLoader {
	public:
		/// @brief キャッシュを初期化します
		void Initialize();

		/// @brief 指定されたファイルパスからモデルデータを読み込む関数,キャッシュに存在する場合はキャッシュから返す,キャッシュに存在しない場合は新たに読み込む
		bool LoadModel(const std::string& filePath, ModelData& modelData);
		/// @brief 指定されたファイルパスからモデルデータを読み込む関数,キャッシュに存在する場合はキャッシュから返す
		ModelData& LoadModel(const std::string& filePath);

		SkinningModelData& LoadSkinningModel(const std::string& filePath);

		/// @brief 指定されたファイルパスからモデルデータを強制的に読み込む関数,キャッシュを無視して新たに読み込む,キャッシュに存在する場合は上書きする
		ModelData& ForceLoadModel(const std::string& filePath);
		/// @brief キャッシュにモデルデータが存在するかどうかを確認する関数
		bool IsModelCached(const std::string& filePath) const;

	private:
		/// @brief 指定されたファイルパスからモデルデータを読み込む関数
		bool LoadModelData(const std::string& filePath, ModelData& modelData);
		/// @brief 指定されたファイルパスからスキニングモデルデータを読み込む関数
		bool LoadSkinningModelData(const std::string& filePath, SkinningModelData& skinningModelData);

		std::unordered_map<std::string, ModelData> modelCache; // モデルデータのキャッシュ
		std::unordered_map<std::string, SkinningModelData> skinningModelCache; // スキニングモデルデータのキャッシュ
		ModelData invalidModelData; // 無効なモデルデータを返すためのメンバ変数
	};

}

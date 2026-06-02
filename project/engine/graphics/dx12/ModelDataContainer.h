/**
 * @file ModelDataContainer.h
 * @brief モデルの頂点バッファリソースを管理するクラス
 */

#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <deque>

#include "common/ModelData.h"
#include "buffer/VertexBuffer.h"

namespace QFE::GRAPHIC::INTERNAL {

	/**
	 * @class ModelDataContainer
	 * @brief 複数の3Dモデルの頂点リソースをGPU上に確保・管理するクラス
	 */
	class ModelDataContainer final {
	public:
		/** @brief 初期化 */
		void Initialize();

		/// @brief モデルデータを読み込んで頂点バッファリソースを作成し、ハンドルを返す
		uint32_t LoadModel(const std::string& path);

		/// @brief ModelDataをハンドルから取得
		const ModelData& GetModelData(uint32_t handle) const;

		/** @brief 終了処理 */
		void Finalize();

	private:
		ModelData invalidModelData_;// エラー回避用の無効なモデルデータ

		std::unordered_map<std::string, uint32_t> loadPathMap_;// モデルのロードパスとハンドルのマップ
		std::deque<ModelData> modelDatas_;// モデルデータのコンテナ
	};

}

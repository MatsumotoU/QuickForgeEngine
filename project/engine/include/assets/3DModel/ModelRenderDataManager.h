/**
 * @file ModelRenderDataManager.h
 * @brief 3Dモデルの描画データを管理するクラス
 */

#pragma once
#include <vector>
#include "Data/ModelData.h"
#include "Data/ModelRenderData.h"

namespace QFE {

	/**
	 * @class ModelRenderDataManager
	 * @brief 読み込まれた3Dモデルの描画用データを一括管理するクラス
	 */
	class ModelRenderDataManager final {
	public:
		ModelRenderDataManager() = default;
		~ModelRenderDataManager() = default;

		/** @brief 初期化 */
		void Initialize();
		/**
		 * @brief 描画データを追加
		 * @param data 追加するデータ
		 * @return データへのハンドル
		 */
		uint32_t Add(const ModelRenderData& data);
		/** @brief ハンドルから描画データを取得 */
		ModelRenderData* Get(uint32_t handle);
		/** @brief 終了処理 */
		void Finalize();

	private:
		std::vector<ModelRenderData> modelRenderDatas_;
	};

}

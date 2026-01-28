/**
 * @file ModelRenderDataManager.cpp
 * @brief 3Dモデルのレンダリングデータ(定数バッファ等のハンドル)を管理するクラスの実装
 */

#include "engine/include/assets/3DModel/ModelRenderDataManager.h"

using namespace QFE;

/** @brief 初期化 */
void ModelRenderDataManager::Initialize() {
	modelRenderDatas_.clear();
}

/** @brief データの追加 */
uint32_t ModelRenderDataManager::Add(const ModelRenderData& data) {
	modelRenderDatas_.push_back(data);
	return static_cast<uint32_t>(modelRenderDatas_.size() - 1);
}

/** @brief データの取得 */
ModelRenderData* ModelRenderDataManager::Get(uint32_t handle) {
	return &modelRenderDatas_.at(handle);
}

/** @brief 終了処理 */
void ModelRenderDataManager::Finalize() {
	modelRenderDatas_.clear();
}

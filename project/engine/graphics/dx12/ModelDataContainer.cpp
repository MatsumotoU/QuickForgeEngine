#include "ModelDataContainer.h"
#include <cassert>

#include "common/PrimitiveVertices.h"
#include "common/AssimpModelLoader.h"

#include "EngineDefines.h"
#include "file/FileUtility.h"

using namespace QFE::GRAPHIC::INTERNAL;

/** @brief 初期化 */
void ModelDataContainer::Initialize() {
	modelDatas_.clear();
	loadPathMap_.clear();
}

uint32_t QFE::GRAPHIC::INTERNAL::ModelDataContainer::LoadModel(const std::string& path) {
    // ファイル名の取得
    std::string fileName = QFE::FILE::GetFileName(path);

    // 同じ名前のモデルが既に存在する場合はそのハンドルを返す
    auto it = loadPathMap_.find(fileName);
    if (it != loadPathMap_.end()) {
		QFE_LOG("Model already loaded: " + path);
		QFE_LOG("Returning existing handle: " + std::to_string(it->second));
        return it->second;
    }

	// モデルデータの読み込み
	ModelData modelData;
	modelData.name = fileName;
	INTERNAL::AssimpModelLoader::LoadModelData(path, modelData);
	modelDatas_.push_back(std::move(modelData));

	// ハンドルの生成とマップへの登録
	uint32_t handle = static_cast<uint32_t>(modelDatas_.size() - 1);
	loadPathMap_[fileName] = handle;

	QFE_LOG("Model loaded: " + path);
	QFE_LOG("Assigned handle: " + std::to_string(handle));
	return handle;
}

const ModelData& QFE::GRAPHIC::INTERNAL::ModelDataContainer::GetModelData(uint32_t handle) const {
	// ハンドルの範囲チェック
	if(modelDatas_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("ModelDataContainer is empty. No model data to retrieve.", SystemError::Abort);
		return invalidModelData_;
	}
	if(modelDatas_.size() <= handle) {
		QFE_REPORT_SYSTEM_ERROR("Invalid model handle: " + std::to_string(handle) + ". Handle exceeds the number of loaded models.", SystemError::Abort);
		return invalidModelData_;
	}

	return modelDatas_.at(handle);
}

/** @brief 終了処理 */
void ModelDataContainer::Finalize() {
	modelDatas_.clear();
	loadPathMap_.clear();
}
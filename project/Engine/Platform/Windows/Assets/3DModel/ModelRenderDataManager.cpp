#include "ModelRenderDataManager.h"

void ModelRenderDataManager::Initialize() {
	modelRenderDatas_.clear();
}

uint32_t ModelRenderDataManager::Add(const ModelRenderData& data) {
	modelRenderDatas_.push_back(data);
	return static_cast<uint32_t>(modelRenderDatas_.size() - 1);
}

ModelRenderData* ModelRenderDataManager::Get(uint32_t handle) {
	return &modelRenderDatas_.at(handle);
}

void ModelRenderDataManager::Finalize() {
	modelRenderDatas_.clear();
}

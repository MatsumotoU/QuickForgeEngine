#pragma once
#include <vector>
#include "Data/ModelData.h"
#include "Data/ModelRenderData.h"

class ModelRenderDataManager final {
public:
	ModelRenderDataManager() = default;
	~ModelRenderDataManager() = default;

	void Initialize();
	uint32_t Add(const ModelRenderData& data);
	const ModelRenderData* Get(uint32_t handle) const;
	void Finalize();

private:
	std::vector<ModelRenderData> modelRenderDatas_;
};
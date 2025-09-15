#pragma once
#include <vector>
#include "Data/ModelData.h"
#include "Graphic/ShaderBuffer/VertexBuffer.h"

class ModelDataManager final {
public:
	ModelDataManager() = default;
	~ModelDataManager() = default;

	void Initialize();
	uint32_t Load(const ModelData);
	void Finalize();

private:
	
	std::vector<ModelData> modelDatas_;
};
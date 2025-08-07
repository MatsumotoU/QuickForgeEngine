#pragma once
#include "Object/Structs/ModelData.h"
#include "Object/Component/Data/MeshComponent.h"

class EngineCore;

class ModelMeshCreater {
public:
	ModelMeshCreater() = default;
	~ModelMeshCreater() = default;
	// モデルコンポーネントのDirectXリソースを構築
	void CreateResourcesFromModelData(EngineCore* engineCore,const std::vector<VertexData>& vertices,MeshComponent& mashComponent);
};
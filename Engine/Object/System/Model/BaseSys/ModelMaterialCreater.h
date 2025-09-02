#pragma once
#include "Object/Component/Data/MaterialComponent.h"
#include "Object/Data/ModelMaterialData.h"

class EngineCore;

class ModelMaterialCreater {
public:
	ModelMaterialCreater() = default;
	~ModelMaterialCreater() = default;

	void CreateMaterialComponent(EngineCore* engineCore,ModelMaterialData& modelMaterialData,MaterialComponent& materialComponent);
};
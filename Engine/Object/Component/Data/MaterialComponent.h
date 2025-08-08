#pragma once
#include <string>
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Shaders/StructsForGpu/Material.h"
#include "Shaders/StructsForGpu/DirectionalLight.h"
#include "Utility/SimpleJson.h"

struct MaterialComponent {
	ConstantBuffer<DirectionalLight> directionalLight_;
	ConstantBuffer<Material> material_;
	std::string textureFilePath;
	uint32_t textureHandle;
};
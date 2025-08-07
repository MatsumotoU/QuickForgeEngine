#pragma once
#include "Math/Transform.h"
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Shaders/StructsForGpu/TransformationMatrix.h"

struct TransformComponent {
	ConstantBuffer<TransformationMatrix> transformationBuffer_;
	Transform transform_;
};
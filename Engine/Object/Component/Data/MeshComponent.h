#pragma once
#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"

struct MeshComponent {
	VertexBuffer<VertexData> vertexBuffers_;
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};
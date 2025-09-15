#pragma once
#include "pch.h"

struct MeshRenderDataHandle {
	uint32_t vertexBufferHandle;
	uint32_t textureHandle;
	uint32_t materialHandle;
	uint32_t wpvBufferHandle;
	uint32_t lightBufferHandle;
};

struct ModelRenderData {
	std::vector<MeshRenderDataHandle> meshRenderDataHandles;
};
#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

struct MeshRenderDataHandle {
	uint32_t vertexBufferHandle;
	uint32_t textureHandle;
	uint32_t materialHandle;
	uint32_t wpvBufferHandle;
	uint32_t lightBufferHandle;
	uint32_t cameraPosBufferHandle;
};

struct ModelRenderData {
	std::vector<MeshRenderDataHandle> meshRenderDataHandles;
};

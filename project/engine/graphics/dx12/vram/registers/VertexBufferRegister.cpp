#include "VertexBufferRegister.h"

using namespace QFE::GRAPHIC::INTERNAL;

QFE::GRAPHIC::VertexBufferHandle VertexBufferRegister::Assign(const std::vector<VertexData>& vertex, const std::string& name) {
	VertexBufferInfo bufferInfo;
	bufferInfo.vertexCount = static_cast<uint32_t>(vertex.size());
	bufferInfo.bufferHandle = info_.createBufferFunc(vertex.size() * sizeof(VertexData));


    return 
}

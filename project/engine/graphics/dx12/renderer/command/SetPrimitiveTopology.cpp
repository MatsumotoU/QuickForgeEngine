#include "SetPrimitiveTopology.h"

using namespace QFE::GRAPHIC;

SetPrimitiveTopology::SetPrimitiveTopology(
	D3D_PRIMITIVE_TOPOLOGY topology) : primitiveTopology_(topology){ }

void SetPrimitiveTopology::Execute(ID3D12GraphicsCommandList* commandList) {
	commandList->IASetPrimitiveTopology(primitiveTopology_);
}

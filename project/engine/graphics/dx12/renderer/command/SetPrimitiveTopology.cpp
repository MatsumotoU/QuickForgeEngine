#include "SetPrimitiveTopology.h"

using namespace QFE::GRAPHIC;

SetPrimitiveTopology::SetPrimitiveTopology(
	D3D_PRIMITIVE_TOPOLOGY topology, std::function<void(D3D_PRIMITIVE_TOPOLOGY)> setPrimitiveTopologyFunc) :
	setPrimitiveTopologyFunc_(setPrimitiveTopologyFunc),
	primitiveTopology_(topology){ }

void SetPrimitiveTopology::Execute() {
	setPrimitiveTopologyFunc_(primitiveTopology_);
}

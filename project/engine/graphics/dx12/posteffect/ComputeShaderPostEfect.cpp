#include "ComputeShaderPostEfect.h"
#include "graphics/D3D12GraphicEngine.h"
#include "core/EngineDefines.h"

void QFE::GRAPHIC::ComputeShaderPostEffect::CreateBuiltInComputePipelineStateObjects(IGraphicEngine* graphicEngine)
{
	D3D12GraphicEngine* d3d12Engine = dynamic_cast<D3D12GraphicEngine*>(graphicEngine);
	if(!d3d12Engine)
	{
		QFE_LOG("ComputeShaderPostEffect::CreateBuiltInComputePipelineStateObjects: graphicEngine is not D3D12GraphicEngine.");
		return;
	}

	d3d12Engine->GetComputePipelineManager()->CreateBuiltInComputePipelineStateObjects();
	
}

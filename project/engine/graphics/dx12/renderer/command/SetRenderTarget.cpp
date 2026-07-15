#include "SetRenderTarget.h"
using namespace QFE::GRAPHIC;

SetRenderTarget::SetRenderTarget(
	RenderTargetHandle renderTarget, DepthStencilHandle depthStencil, 
	std::function<void(RenderTargetHandle, DepthStencilHandle)> setRenderTargetFunc) :
	renderTargetHandle_(renderTarget), depthStencilHandle_(depthStencil), 
	setRenderTargetFunc_(std::move(setRenderTargetFunc)){ }

void SetRenderTarget::Execute(ID3D12GraphicsCommandList* commandList) {
	setRenderTargetFunc_(renderTargetHandle_, depthStencilHandle_);
}
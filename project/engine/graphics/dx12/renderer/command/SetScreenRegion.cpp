#include "SetScreenRegion.h"

using namespace QFE::GRAPHIC;

SetScreenRegion::SetScreenRegion(
	ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
	std::function<void(ViewPortHandle)> setViewportFunc,
	std::function<void(ScissorRectHandle)> setScissorRectFunc) : 
	viewportHandle_(viewportHandle),
	scissorRectHandle_(scissorRectHandle),
	setViewportFunc_(std::move(setViewportFunc)),
	setScissorRectFunc_(std::move(setScissorRectFunc))
{
}

void SetScreenRegion::Execute() {
	setViewportFunc_(viewportHandle_);
	setScissorRectFunc_(scissorRectHandle_);
}

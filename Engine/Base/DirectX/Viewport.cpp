#include "Viewport.h"
#include "../Windows/WinApp.h"
#include <cassert>

void ViewPort::Inititalize(WinApp* winApp) {
	viewport_ = {};
	viewport_.Width = winApp->kWindowWidth;
	viewport_.Height = winApp->kWindowHeight;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}

void ViewPort::SetWidth(const FLOAT& width) {
	viewport_.Width = width;
	assert(width <= 0.0f);
}

void ViewPort::SetHeight(const FLOAT& height) {
	viewport_.Height = height;
	assert(height <= 0.0f);
}

void ViewPort::SetMinDepth(const FLOAT& minDepth) {
	viewport_.MinDepth = minDepth;
	assert(minDepth < viewport_.MaxDepth);
}

void ViewPort::SetMaxDepth(const FLOAT& maxDepth) {
	viewport_.MaxDepth = maxDepth;
	assert(maxDepth > viewport_.MinDepth);
}

const D3D12_VIEWPORT* ViewPort::GetViewport() {
	return &viewport_;
}

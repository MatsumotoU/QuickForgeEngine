#include "ScissorRect.h"
#include "../Windows/WinApp.h"

void ScissorRect::Initialize(WinApp* winApp) {
	scissorRect_.left = 0;
	scissorRect_.right = winApp->kWindowWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = winApp->kWindowHeight;
}

D3D12_RECT* ScissorRect::GetScissorRect() {
	return &scissorRect_;
}

void ScissorRect::SetRight(const int32_t& right) {
	scissorRect_.right = right;
}

void ScissorRect::SetBottom(const int32_t& bottom) {
	scissorRect_.bottom = bottom;
}

#include "GraphicCommon.h"

GraphicCommon::GraphicCommon(HWND hwnd, uint32_t width, uint32_t height) {
	directXCommon_.Initialize(hwnd, width, height);
}

void GraphicCommon::Initialize() {
}

void GraphicCommon::PreDraw() {
}

void GraphicCommon::PostDraw() {
}

void GraphicCommon::Shutdown() {
}

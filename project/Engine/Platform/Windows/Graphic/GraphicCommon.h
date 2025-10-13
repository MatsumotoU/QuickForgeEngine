#pragma once
#include "Core/Graphic/IGraphicCommon.h"

#include "DirectXCommon/DirectXCommon.h"

class GraphicCommon final : public IGraphicCommon {
public:
	GraphicCommon(HWND hwnd, uint32_t width, uint32_t height);
	~GraphicCommon() override = default;
	void Initialize() override;
	void PreDraw() override;
	void PostDraw() override;
	void Shutdown() override;
private:
	DirectXCommon directXCommon_;
};
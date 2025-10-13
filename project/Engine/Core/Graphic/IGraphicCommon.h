#pragma once
class IGraphicCommon {
public:
	IGraphicCommon() = default;
	virtual ~IGraphicCommon() = default;

	virtual void Initialize() = 0;
	virtual void PreDraw() = 0;
	virtual void PostDraw() = 0;
	virtual void Shutdown() = 0;
};
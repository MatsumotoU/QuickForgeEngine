#pragma once
#include "../../Math/Vector/Vector3.h"
#include "Base/DirectX/WVPResource.h"
#include "Base/DirectX/MaterialResource.h"

class EngineCore;
class PipelineStateObject;

static inline const uint32_t kGraphRendererMaxTriangleCount = 512;
static inline const uint32_t kGraphRendererMaxLineCount = 512;
static inline const uint32_t kGraphRendererMaxPointCount = 512;

class GraphRenderer {
public:
	void Initialize(EngineCore* engineCore);

	void PreDraw();

	void PostDraw();

public:
	void DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, Vector4 color);

private:
	uint32_t triangleIndex_;
	uint32_t lineIndex_;
	uint32_t pointIndex_;

private:
	WVPResource triangleWvp_;
	WVPResource lineWvp_;
	WVPResource pointWvp_;
	MaterialResource material_;

private:
	EngineCore* engineCore_;
	PipelineStateObject* trianglePso_;
	PipelineStateObject* linePso_;
	PipelineStateObject* pointPso_;
};
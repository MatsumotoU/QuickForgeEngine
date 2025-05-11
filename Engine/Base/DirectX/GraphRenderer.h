#pragma once
#include "../../Math/Vector/Vector3.h"

class DirectXCommon;

class GraphRenderer {
public:
	void Initialize(DirectXCommon* dxCommon);

public:
	void DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3,uint32_t color);

private:
	DirectXCommon* dxCommon_;
	
};
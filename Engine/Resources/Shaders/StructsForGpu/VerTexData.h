#pragma once
#include "Math/Vector/Vector4.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector2.h"

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;// uv座標系
	Vector3 normal;
};
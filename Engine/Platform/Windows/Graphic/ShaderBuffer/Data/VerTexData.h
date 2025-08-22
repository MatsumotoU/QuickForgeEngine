#pragma once
#include "Core/Math/Vector/Vector4.h"
#include "Core/Math/Vector/Vector3.h"
#include "Core/Math/Vector/Vector2.h"

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;// uv座標系
	Vector3 normal;
};
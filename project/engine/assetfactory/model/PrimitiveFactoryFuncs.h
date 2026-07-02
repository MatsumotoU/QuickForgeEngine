#pragma once
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

namespace QFE::ASSET {
	/// @brief プリミティブ形状のメッシュデータを生成する関数群
	std::vector<VertexData> CreatePlane(float width = 1.0f, float height = 1.0f, uint32_t segmentsX = 1, uint32_t segmentsY = 1, bool invertFace = false);
	/// @brief CreateBox 関数は、単位立方体のメッシュデータを生成します。
	std::vector<VertexData> CreateBox(bool invertFace = false);
	/// @brief CreateRing 関数は、リング状のメッシュデータを生成します。
	std::vector<VertexData> CreateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);
	/// @brief シリンダーのメッシュデータを生成します。
	std::vector<VertexData> CreateCylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32, bool invertFace = false);
}

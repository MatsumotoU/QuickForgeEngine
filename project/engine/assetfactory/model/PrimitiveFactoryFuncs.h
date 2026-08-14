#pragma once
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include <string>
#include <vector>

namespace QFE::ASSET {
	/// @brief エディタで選択可能な組み込みプリミティブ名を取得します。
	const std::vector<std::string>& GetPrimitiveMeshNames();
	/// @brief 名前が組み込みプリミティブか判定します。
	bool IsPrimitiveMeshName(const std::string& name);
	/// @brief 組み込み名からプリミティブメッシュを生成します。
	std::vector<VertexData> CreatePrimitiveMesh(const std::string& name);

	/// @brief プリミティブ形状のメッシュデータを生成する関数群
	std::vector<VertexData> CreatePlane(float width = 1.0f, float height = 1.0f, uint32_t segmentsX = 1, uint32_t segmentsY = 1, bool invertFace = false);
	/// @brief CreateBox 関数は、単位立方体のメッシュデータを生成します。
	std::vector<VertexData> CreateBox(bool invertFace = false);
	/// @brief CreateRing 関数は、リング状のメッシュデータを生成します。
	std::vector<VertexData> CreateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);
	/// @brief シリンダーのメッシュデータを生成します。
	std::vector<VertexData> CreateCylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32, bool invertFace = false);
	std::vector<VertexData> CreateSphere(float radius = 0.5f, uint32_t slices = 32, uint32_t stacks = 16, bool invertFace = false);
	std::vector<VertexData> CreateCone(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32, bool invertFace = false);
	std::vector<VertexData> CreateDisk(float radius = 0.5f, uint32_t segments = 32, bool invertFace = false);
	std::vector<VertexData> CreateTorus(float majorRadius = 0.35f, float minorRadius = 0.15f, uint32_t majorSegments = 32, uint32_t minorSegments = 16, bool invertFace = false);
	std::vector<VertexData> CreateCapsule(float radius = 0.25f, float cylinderHeight = 0.5f, uint32_t slices = 32, uint32_t hemisphereStacks = 8, bool invertFace = false);
}

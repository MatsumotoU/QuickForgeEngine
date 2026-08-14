#include "PrimitiveFactoryFuncs.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace QFE::ASSET {

namespace {
	void ApplyFaceDirection(std::vector<VertexData>& vertices, bool invertFace) {
		if (!invertFace) return;
		for (VertexData& vertex : vertices) {
			vertex.normal = -vertex.normal;
		}
		for (size_t index = 0; index + 2 < vertices.size(); index += 3) {
			std::swap(vertices[index], vertices[index + 2]);
		}
	}

	VertexData MakeVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) {
		VertexData vertex;
		vertex.position = { x, y, z, 1.0f };
		vertex.texcoord = { u, v };
		vertex.normal = { nx, ny, nz };
		return vertex;
	}
}

const std::vector<std::string>& GetPrimitiveMeshNames() {
	static const std::vector<std::string> names{
		"Primitive/Quad",
		"Primitive/Plane",
		"Primitive/Box",
		"Primitive/Sphere",
		"Primitive/Cylinder",
		"Primitive/Cone",
		"Primitive/Disk",
		"Primitive/Ring",
		"Primitive/Torus",
		"Primitive/Capsule"
	};
	return names;
}

bool IsPrimitiveMeshName(const std::string& name) {
	const auto& names = GetPrimitiveMeshNames();
	return std::find(names.begin(), names.end(), name) != names.end();
}

std::vector<VertexData> CreatePrimitiveMesh(const std::string& name) {
	if (name == "Primitive/Quad" || name == "Primitive/Plane") return CreatePlane();
	if (name == "Primitive/Box") return CreateBox();
	if (name == "Primitive/Sphere") return CreateSphere();
	if (name == "Primitive/Cylinder") return CreateCylinder();
	if (name == "Primitive/Cone") return CreateCone();
	if (name == "Primitive/Disk") return CreateDisk();
	if (name == "Primitive/Ring") return CreateRing();
	if (name == "Primitive/Torus") return CreateTorus();
	if (name == "Primitive/Capsule") return CreateCapsule();
	return {};
}

std::vector<VertexData> CreatePlane(float width, float height, uint32_t segmentsX, uint32_t segmentsY, bool invertFace) {
	std::vector<VertexData> planeMesh(segmentsX * segmentsY * 6);

	// 頂点データを設定
	for (uint32_t y = 0; y < segmentsY; ++y) {
		for (uint32_t x = 0; x < segmentsX; ++x) {
			uint32_t index = (y * segmentsX + x) * 6;
			float x0 = (x / static_cast<float>(segmentsX)) * width - width / 2.0f;
			float x1 = ((x + 1) / static_cast<float>(segmentsX)) * width - width / 2.0f;
			float y0 = (y / static_cast<float>(segmentsY)) * height - height / 2.0f;
			float y1 = ((y + 1) / static_cast<float>(segmentsY)) * height - height / 2.0f;
			// First triangle
			planeMesh[index].position = { x0, y0, 0.0f, 1.0f }; planeMesh[index].texcoord = { 0.0f, 1.0f }; planeMesh[index].normal = { 0.0f, 0.0f, -1.0f };
			planeMesh[index + 1].position = { x1, y1, 0.0f, 1.0f }; planeMesh[index + 1].texcoord = { 1.0f, 0.0f }; planeMesh[index + 1].normal = { 0.0f, 0.0f, -1.0f };
			planeMesh[index + 2].position = { x1, y0, 0.0f, 1.0f }; planeMesh[index + 2].texcoord = { 1.0f, 1.0f }; planeMesh[index + 2].normal = { 0.0f, 0.0f, -1.0f };
			// Second triangle
			planeMesh[index + 3].position = { x0, y0, 0.0f, 1.0f }; planeMesh[index + 3].texcoord = { 0.0f, 1.0f }; planeMesh[index + 3].normal = { 0.0f, 0.0f, -1.0f };
			planeMesh[index + 4].position = { x0, y1, 0.0f, 1.0f }; planeMesh[index + 4].texcoord = { 0.0f, 0.0f }; planeMesh[index + 4].normal = { 0.0f, 0.0f, -1.0f };
			planeMesh[index + 5].position = { x1, y1, 0.0f, 1.0f }; planeMesh[index + 5].texcoord = { 1.0f, 0.0f }; planeMesh[index + 5].normal = { 0.0f, 0.0f, -1.0f };
		}
	}

	ApplyFaceDirection(planeMesh, invertFace);
	return planeMesh;
}

std::vector<VertexData> CreateBox(bool invertFace) {
	std::vector<VertexData> boxMesh(36);

	// 頂点データを設定
	// Front face (Z = -0.5)
	boxMesh[0].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[0].texcoord = { 0.0f, 1.0f }; boxMesh[0].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh[1].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[1].texcoord = { 0.0f, 0.0f }; boxMesh[1].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh[2].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[2].texcoord = { 1.0f, 0.0f }; boxMesh[2].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh[3].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[3].texcoord = { 0.0f, 1.0f }; boxMesh[3].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh[4].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[4].texcoord = { 1.0f, 0.0f }; boxMesh[4].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh[5].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[5].texcoord = { 1.0f, 1.0f }; boxMesh[5].normal = { 0.0f, 0.0f, -1.0f };

	// Back face (Z = 0.5)
	boxMesh[6].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[6].texcoord = { 0.0f, 1.0f }; boxMesh[6].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh[7].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh[7].texcoord = { 0.0f, 0.0f }; boxMesh[7].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh[8].position = { -0.5f,  0.5f,  0.5f, 1.0f }; boxMesh[8].texcoord = { 1.0f, 0.0f }; boxMesh[8].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh[9].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[9].texcoord = { 0.0f, 1.0f }; boxMesh[9].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh[10].position = { -0.5f,  0.5f, 0.5f, 1.0f }; boxMesh[10].texcoord = { 1.0f, 0.0f }; boxMesh[10].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh[11].position = { -0.5f, -0.5f, 0.5f, 1.0f }; boxMesh[11].texcoord = { 1.0f, 1.0f }; boxMesh[11].normal = { 0.0f, 0.0f, 1.0f };

	// Left face (X = -0.5)
	boxMesh[12].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[12].texcoord = { 0.0f, 1.0f }; boxMesh[12].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh[13].position = { -0.5f,  0.5f,  0.5f, 1.0f }; boxMesh[13].texcoord = { 0.0f, 0.0f }; boxMesh[13].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh[14].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[14].texcoord = { 1.0f, 0.0f }; boxMesh[14].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh[15].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[15].texcoord = { 0.0f, 1.0f }; boxMesh[15].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh[16].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[16].texcoord = { 1.0f, 0.0f }; boxMesh[16].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh[17].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[17].texcoord = { 1.0f, 1.0f }; boxMesh[17].normal = { -1.0f, 0.0f, 0.0f };

	// Right face (X = 0.5)
	boxMesh[18].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[18].texcoord = { 0.0f, 1.0f }; boxMesh[18].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh[19].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh[19].texcoord = { 0.0f, 0.0f }; boxMesh[19].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh[20].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh[20].texcoord = { 1.0f, 0.0f }; boxMesh[20].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh[21].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[21].texcoord = { 0.0f, 1.0f }; boxMesh[21].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh[22].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh[22].texcoord = { 1.0f, 0.0f }; boxMesh[22].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh[23].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[23].texcoord = { 1.0f, 1.0f }; boxMesh[23].normal = { 1.0f, 0.0f, 0.0f };

	// Top face (Y = 0.5)
	boxMesh[24].position = { -0.5f, 0.5f, -0.5f, 1.0f }; boxMesh[24].texcoord = { 0.0f, 1.0f }; boxMesh[24].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh[25].position = { -0.5f, 0.5f,  0.5f, 1.0f }; boxMesh[25].texcoord = { 0.0f, 0.0f }; boxMesh[25].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh[26].position = { 0.5f, 0.5f,  0.5f, 1.0f }; boxMesh[26].texcoord = { 1.0f, 0.0f }; boxMesh[26].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh[27].position = { -0.5f, 0.5f, -0.5f, 1.0f }; boxMesh[27].texcoord = { 0.0f, 1.0f }; boxMesh[27].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh[28].position = { 0.5f, 0.5f,  0.5f, 1.0f }; boxMesh[28].texcoord = { 1.0f, 0.0f }; boxMesh[28].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh[29].position = { 0.5f, 0.5f, -0.5f, 1.0f }; boxMesh[29].texcoord = { 1.0f, 1.0f }; boxMesh[29].normal = { 0.0f, 1.0f, 0.0f };

	// Bottom face (Y = -0.5)
	boxMesh[30].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[30].texcoord = { 0.0f, 1.0f }; boxMesh[30].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh[31].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[31].texcoord = { 0.0f, 0.0f }; boxMesh[31].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh[32].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[32].texcoord = { 1.0f, 0.0f }; boxMesh[32].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh[33].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[33].texcoord = { 0.0f, 1.0f }; boxMesh[33].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh[34].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh[34].texcoord = { 1.0f, 0.0f }; boxMesh[34].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh[35].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh[35].texcoord = { 1.0f, 1.0f }; boxMesh[35].normal = { 0.0f, -1.0f, 0.0f };

	// 面を反転させるかどうかの処理
	if (invertFace) {
		// 法線を反転させる
		for (size_t i = 0; i < boxMesh.size(); ++i) {
			boxMesh[i].normal = -boxMesh[i].normal;
		}
		// 頂点の順序を反転させる
		for (size_t i = 0; i < boxMesh.size(); i += 3) {
			std::swap(boxMesh[i], boxMesh[i + 2]);
		}
	}

	return boxMesh;
}

std::vector<VertexData> CreateRing(float innerRadius, float outerRadius, uint32_t segments, bool invertFace) {
	std::vector<VertexData> ringMesh(segments * 6);

	// 頂点データを設定
	for (uint32_t i = 0; i < segments; ++i) {
		float angle = (2.0f * 3.14159265f * i) / segments;
		float nextAngle = (2.0f * 3.14159265f * (i + 1)) / segments;

		VertexData iCurr, iNext, oCurr, oNext;

		// 内側の現在の頂点
		iCurr.position = { innerRadius * cos(angle), innerRadius * sin(angle), 0.0f, 1.0f };
		iCurr.texcoord = { static_cast<float>(i) / segments, 1.0f };
		iCurr.normal = { 0.0f, 0.0f, -1.0f };

		// 外側の現在の頂点
		oCurr.position = { outerRadius * cos(angle), outerRadius * sin(angle), 0.0f, 1.0f };
		oCurr.texcoord = { static_cast<float>(i) / segments, 0.0f };
		oCurr.normal = { 0.0f, 0.0f, -1.0f };

		// 内側の次の頂点
		iNext.position = { innerRadius * cos(nextAngle), innerRadius * sin(nextAngle), 0.0f, 1.0f };
		iNext.texcoord = { static_cast<float>(i + 1) / segments, 1.0f };
		iNext.normal = { 0.0f, 0.0f, -1.0f };

		// 外側の次の頂点
		oNext.position = { outerRadius * cos(nextAngle), outerRadius * sin(nextAngle), 0.0f, 1.0f };
		oNext.texcoord = { static_cast<float>(i + 1) / segments, 0.0f };
		oNext.normal = { 0.0f, 0.0f, -1.0f };

		// 三角形1
		ringMesh[i * 6 + 0] = iCurr;
		ringMesh[i * 6 + 1] = iNext;
		ringMesh[i * 6 + 2] = oCurr;

		// 三角形2
		ringMesh[i * 6 + 3] = iNext;
		ringMesh[i * 6 + 4] = oNext;
		ringMesh[i * 6 + 5] = oCurr;
	}

	// 面を反転させるかどうかの処理
	if (invertFace) {
		// 法線を反転させる
		for (size_t i = 0; i < ringMesh.size(); ++i) {
			ringMesh[i].normal = -ringMesh[i].normal;
		}
		// 頂点の順序を反転させる
		for (size_t i = 0; i < ringMesh.size(); i += 3) {
			std::swap(ringMesh[i], ringMesh[i + 2]);
		}
	}

	return ringMesh;
}

std::vector<VertexData> CreateCylinder(float radius, float height, uint32_t segments, bool invertFace) {
	// 1セグメントあたり: 側面(6頂点) + 天面(3頂点) + 底面(3頂点) = 12頂点
	std::vector<VertexData> cylinderMesh(segments * 12);
	float halfHeight = height / 2.0f;

	for (uint32_t i = 0; i < segments; ++i) {
		float angle = (2.0f * 3.14159265f * i) / segments;
		float nextAngle = (2.0f * 3.14159265f * (i + 1)) / segments;

		float cosCurr = cos(angle);
		float sinCurr = sin(angle);
		float cosNext = cos(nextAngle);
		float sinNext = sin(nextAngle);

		// --- 頂点データの準備 ---

		// 側面用の頂点（法線は外側を向く）
		VertexData sideBotCurr, sideBotNext, sideTopCurr, sideTopNext;
		sideBotCurr.position = { radius * cosCurr, -halfHeight, radius * sinCurr };
		sideBotCurr.normal = { cosCurr, 0.0f, sinCurr };
		sideBotCurr.texcoord = { static_cast<float>(i) / segments, 1.0f };

		sideBotNext.position = { radius * cosNext, -halfHeight, radius * sinNext };
		sideBotNext.normal = { cosNext, 0.0f, sinNext };
		sideBotNext.texcoord = { static_cast<float>(i + 1) / segments, 1.0f };

		sideTopCurr.position = { radius * cosCurr, halfHeight, radius * sinCurr };
		sideTopCurr.normal = { cosCurr, 0.0f, sinCurr };
		sideTopCurr.texcoord = { static_cast<float>(i) / segments, 0.0f };

		sideTopNext.position = { radius * cosNext, halfHeight, radius * sinNext };
		sideTopNext.normal = { cosNext, 0.0f, sinNext };
		sideTopNext.texcoord = { static_cast<float>(i + 1) / segments, 0.0f };

		// 天面用の頂点（法線は真上、UVは簡易的に0.5中心）
		VertexData topCenter, topCurr, topNext;
		topCenter.position = { 0.0f, halfHeight, 0.0f };
		topCenter.normal = { 0.0f, 1.0f, 0.0f };
		topCenter.texcoord = { 0.5f, 0.5f };

		topCurr = sideTopCurr;
		topCurr.normal = { 0.0f, 1.0f, 0.0f }; // 法線だけ真上に上書き

		topNext = sideTopNext;
		topNext.normal = { 0.0f, 1.0f, 0.0f }; // 法線だけ真上に上書き

		// 底面用の頂点（法線は真下）
		VertexData botCenter, botCurr, botNext;
		botCenter.position = { 0.0f, -halfHeight, 0.0f };
		botCenter.normal = { 0.0f, -1.0f, 0.0f };
		botCenter.texcoord = { 0.5f, 0.5f };

		botCurr = sideBotCurr;
		botCurr.normal = { 0.0f, -1.0f, 0.0f }; // 法線だけ真下に上書き

		botNext = sideBotNext;
		botNext.normal = { 0.0f, -1.0f, 0.0f }; // 法線だけ真下に上書き


		// --- メッシュへの代入 (1セグメント = 12頂点) ---
		size_t offset = i * 12;

		// 1. 側面
		cylinderMesh[offset + 0] = sideBotCurr;
		cylinderMesh[offset + 1] = sideTopNext;
		cylinderMesh[offset + 2] = sideBotNext;

		cylinderMesh[offset + 3] = sideBotCurr;
		cylinderMesh[offset + 4] = sideTopCurr;
		cylinderMesh[offset + 5] = sideTopNext;

		// 2. 天面
		cylinderMesh[offset + 6] = topCenter;
		cylinderMesh[offset + 7] = topNext;
		cylinderMesh[offset + 8] = topCurr;

		// 3. 底面
		cylinderMesh[offset + 9] = botCenter;
		cylinderMesh[offset + 10] = botCurr;
		cylinderMesh[offset + 11] = botNext;
	}

	// --- 面を反転させるかどうかの処理（リングの実装と完全同一） ---
	if (invertFace) {
		// 法線を反転させる
		for (size_t i = 0; i < cylinderMesh.size(); ++i) {
			cylinderMesh[i].normal = { -cylinderMesh[i].normal.x, -cylinderMesh[i].normal.y, -cylinderMesh[i].normal.z };
		}
		// 頂点の順序を反転させる（巻き順を時計回りに）
		for (size_t i = 0; i < cylinderMesh.size(); i += 3) {
			std::swap(cylinderMesh[i], cylinderMesh[i + 2]);
		}
	}

	return cylinderMesh;
}

std::vector<VertexData> CreateSphere(float radius, uint32_t slices, uint32_t stacks, bool invertFace) {
	slices = std::max(slices, 3u);
	stacks = std::max(stacks, 2u);
	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(slices) * stacks * 6);
	constexpr float pi = std::numbers::pi_v<float>;

	auto makeSphereVertex = [&](uint32_t slice, uint32_t stack) {
		const float u = static_cast<float>(slice) / slices;
		const float v = static_cast<float>(stack) / stacks;
		const float longitude = u * 2.0f * pi;
		const float latitude = (v - 0.5f) * pi;
		const float cosLatitude = std::cos(latitude);
		const float nx = cosLatitude * std::cos(longitude);
		const float ny = std::sin(latitude);
		const float nz = cosLatitude * std::sin(longitude);
		return MakeVertex(radius * nx, radius * ny, radius * nz, u, 1.0f - v, nx, ny, nz);
	};

	for (uint32_t stack = 0; stack < stacks; ++stack) {
		for (uint32_t slice = 0; slice < slices; ++slice) {
			const VertexData v00 = makeSphereVertex(slice, stack);
			const VertexData v10 = makeSphereVertex(slice + 1, stack);
			const VertexData v01 = makeSphereVertex(slice, stack + 1);
			const VertexData v11 = makeSphereVertex(slice + 1, stack + 1);
			vertices.insert(vertices.end(), { v00, v01, v11, v00, v11, v10 });
		}
	}
	ApplyFaceDirection(vertices, invertFace);
	return vertices;
}

std::vector<VertexData> CreateCone(float radius, float height, uint32_t segments, bool invertFace) {
	segments = std::max(segments, 3u);
	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(segments) * 6);
	const float halfHeight = height * 0.5f;
	const float slope = radius / std::max(height, 0.0001f);
	constexpr float pi = std::numbers::pi_v<float>;

	for (uint32_t segment = 0; segment < segments; ++segment) {
		const float u0 = static_cast<float>(segment) / segments;
		const float u1 = static_cast<float>(segment + 1) / segments;
		const float angle0 = u0 * 2.0f * pi;
		const float angle1 = u1 * 2.0f * pi;
		const float c0 = std::cos(angle0);
		const float s0 = std::sin(angle0);
		const float c1 = std::cos(angle1);
		const float s1 = std::sin(angle1);
		const float normalLength = std::sqrt(1.0f + slope * slope);

		const VertexData bottom0 = MakeVertex(radius * c0, -halfHeight, radius * s0, u0, 1.0f, c0 / normalLength, slope / normalLength, s0 / normalLength);
		const VertexData bottom1 = MakeVertex(radius * c1, -halfHeight, radius * s1, u1, 1.0f, c1 / normalLength, slope / normalLength, s1 / normalLength);
		const VertexData apex = MakeVertex(0.0f, halfHeight, 0.0f, (u0 + u1) * 0.5f, 0.0f, (c0 + c1) * 0.5f / normalLength, slope / normalLength, (s0 + s1) * 0.5f / normalLength);
		vertices.insert(vertices.end(), { bottom0, apex, bottom1 });

		const VertexData center = MakeVertex(0.0f, -halfHeight, 0.0f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f);
		const VertexData cap0 = MakeVertex(radius * c0, -halfHeight, radius * s0, c0 * 0.5f + 0.5f, s0 * 0.5f + 0.5f, 0.0f, -1.0f, 0.0f);
		const VertexData cap1 = MakeVertex(radius * c1, -halfHeight, radius * s1, c1 * 0.5f + 0.5f, s1 * 0.5f + 0.5f, 0.0f, -1.0f, 0.0f);
		vertices.insert(vertices.end(), { center, cap0, cap1 });
	}
	ApplyFaceDirection(vertices, invertFace);
	return vertices;
}

std::vector<VertexData> CreateDisk(float radius, uint32_t segments, bool invertFace) {
	segments = std::max(segments, 3u);
	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(segments) * 3);
	constexpr float pi = std::numbers::pi_v<float>;

	for (uint32_t segment = 0; segment < segments; ++segment) {
		const float angle0 = static_cast<float>(segment) / segments * 2.0f * pi;
		const float angle1 = static_cast<float>(segment + 1) / segments * 2.0f * pi;
		const float c0 = std::cos(angle0);
		const float s0 = std::sin(angle0);
		const float c1 = std::cos(angle1);
		const float s1 = std::sin(angle1);
		vertices.push_back(MakeVertex(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f));
		vertices.push_back(MakeVertex(radius * c1, radius * s1, 0.0f, c1 * 0.5f + 0.5f, 0.5f - s1 * 0.5f, 0.0f, 0.0f, -1.0f));
		vertices.push_back(MakeVertex(radius * c0, radius * s0, 0.0f, c0 * 0.5f + 0.5f, 0.5f - s0 * 0.5f, 0.0f, 0.0f, -1.0f));
	}
	ApplyFaceDirection(vertices, invertFace);
	return vertices;
}

std::vector<VertexData> CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments, bool invertFace) {
	majorSegments = std::max(majorSegments, 3u);
	minorSegments = std::max(minorSegments, 3u);
	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(majorSegments) * minorSegments * 6);
	constexpr float pi = std::numbers::pi_v<float>;

	auto makeTorusVertex = [&](uint32_t majorIndex, uint32_t minorIndex) {
		const float u = static_cast<float>(majorIndex) / majorSegments;
		const float v = static_cast<float>(minorIndex) / minorSegments;
		const float majorAngle = u * 2.0f * pi;
		const float minorAngle = v * 2.0f * pi;
		const float cm = std::cos(majorAngle);
		const float sm = std::sin(majorAngle);
		const float ct = std::cos(minorAngle);
		const float st = std::sin(minorAngle);
		const float ringRadius = majorRadius + minorRadius * ct;
		return MakeVertex(ringRadius * cm, minorRadius * st, ringRadius * sm, u, v, ct * cm, st, ct * sm);
	};

	for (uint32_t majorIndex = 0; majorIndex < majorSegments; ++majorIndex) {
		for (uint32_t minorIndex = 0; minorIndex < minorSegments; ++minorIndex) {
			const VertexData v00 = makeTorusVertex(majorIndex, minorIndex);
			const VertexData v10 = makeTorusVertex(majorIndex + 1, minorIndex);
			const VertexData v01 = makeTorusVertex(majorIndex, minorIndex + 1);
			const VertexData v11 = makeTorusVertex(majorIndex + 1, minorIndex + 1);
			vertices.insert(vertices.end(), { v00, v01, v11, v00, v11, v10 });
		}
	}
	ApplyFaceDirection(vertices, invertFace);
	return vertices;
}

std::vector<VertexData> CreateCapsule(float radius, float cylinderHeight, uint32_t slices, uint32_t hemisphereStacks, bool invertFace) {
	slices = std::max(slices, 3u);
	hemisphereStacks = std::max(hemisphereStacks, 2u);
	const uint32_t stacks = hemisphereStacks * 2;
	const float halfCylinder = cylinderHeight * 0.5f;
	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(slices) * stacks * 6);
	constexpr float pi = std::numbers::pi_v<float>;

	auto makeCapsuleVertex = [&](uint32_t slice, uint32_t stack) {
		const float u = static_cast<float>(slice) / slices;
		const float v = static_cast<float>(stack) / stacks;
		const float longitude = u * 2.0f * pi;
		const float latitude = (v - 0.5f) * pi;
		const float cosLatitude = std::cos(latitude);
		const float nx = cosLatitude * std::cos(longitude);
		const float ny = std::sin(latitude);
		const float nz = cosLatitude * std::sin(longitude);
		const float yOffset = ny >= 0.0f ? halfCylinder : -halfCylinder;
		return MakeVertex(radius * nx, radius * ny + yOffset, radius * nz, u, 1.0f - v, nx, ny, nz);
	};

	for (uint32_t stack = 0; stack < stacks; ++stack) {
		for (uint32_t slice = 0; slice < slices; ++slice) {
			const VertexData v00 = makeCapsuleVertex(slice, stack);
			const VertexData v10 = makeCapsuleVertex(slice + 1, stack);
			const VertexData v01 = makeCapsuleVertex(slice, stack + 1);
			const VertexData v11 = makeCapsuleVertex(slice + 1, stack + 1);
			vertices.insert(vertices.end(), { v00, v01, v11, v00, v11, v10 });
		}
	}
	ApplyFaceDirection(vertices, invertFace);
	return vertices;
}

} // namespace QFE::GRAPHIC

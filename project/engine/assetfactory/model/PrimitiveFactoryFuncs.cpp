#include "PrimitiveFactoryFuncs.h"

namespace QFE::ASSET {

std::vector<VertexData> CreatePlane(float width, float height, uint32_t segmentsX, uint32_t segmentsY, bool invertFace) {
	std::vector<VertexData> planeMesh(segmentsX * segmentsY * 6);
	for (uint32_t i = 0; i < segmentsX * segmentsY * 6; ++i) {
		planeMesh.push_back(VertexData());
	}

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

	// 法線を反転させる
	if (invertFace) {
		for (auto& vertex : planeMesh) {
			vertex.normal = -vertex.normal;
		}
	}
	return planeMesh;
}

std::vector<VertexData> CreateBox(bool invertFace) {
	std::vector<VertexData> boxMesh(36);
	for (int i = 0; i < 36; ++i) {
		boxMesh.push_back(VertexData());
	}

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
	for (uint32_t i = 0; i < segments * 6; ++i) {
		ringMesh.push_back(VertexData());
	}

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

		// 三角形1 (内側現在、外側現在、内側次)
		ringMesh[i * 6 + 0] = iCurr;
		ringMesh[i * 6 + 1] = oCurr;
		ringMesh[i * 6 + 2] = iNext;

		// 三角形2 (内側次、外側現在、外側次)
		ringMesh[i * 6 + 3] = iNext;
		ringMesh[i * 6 + 4] = oCurr;
		ringMesh[i * 6 + 5] = oNext;
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

} // namespace QFE::GRAPHIC

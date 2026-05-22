#include "engine/include/assets/3DModel/PrimitiveVertices.h"

QFE::MeshData QFE::PRIMITIVE::CreateBox(bool invertFace)
{
	MeshData boxMesh(36);
	for(int i = 0; i < 36; ++i) {
		boxMesh.vertices.push_back(VertexData());
	}

	// 頂点データを設定
	// Front face (Z = -0.5)
	boxMesh.vertices[0].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[0].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[0].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh.vertices[1].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[1].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[1].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh.vertices[2].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[2].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[2].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh.vertices[3].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[3].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[3].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh.vertices[4].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[4].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[4].normal = { 0.0f, 0.0f, -1.0f };
	boxMesh.vertices[5].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[5].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[5].normal = { 0.0f, 0.0f, -1.0f };

	// Back face (Z = 0.5)
	boxMesh.vertices[6].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[6].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[6].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh.vertices[7].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh.vertices[7].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[7].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh.vertices[8].position = { -0.5f,  0.5f,  0.5f, 1.0f }; boxMesh.vertices[8].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[8].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh.vertices[9].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[9].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[9].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh.vertices[10].position = { -0.5f,  0.5f, 0.5f, 1.0f }; boxMesh.vertices[10].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[10].normal = { 0.0f, 0.0f, 1.0f };
	boxMesh.vertices[11].position = { -0.5f, -0.5f, 0.5f, 1.0f }; boxMesh.vertices[11].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[11].normal = { 0.0f, 0.0f, 1.0f };

	// Left face (X = -0.5)
	boxMesh.vertices[12].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[12].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[12].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh.vertices[13].position = { -0.5f,  0.5f,  0.5f, 1.0f }; boxMesh.vertices[13].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[13].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh.vertices[14].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[14].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[14].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh.vertices[15].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[15].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[15].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh.vertices[16].position = { -0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[16].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[16].normal = { -1.0f, 0.0f, 0.0f };
	boxMesh.vertices[17].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[17].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[17].normal = { -1.0f, 0.0f, 0.0f };

	// Right face (X = 0.5)
	boxMesh.vertices[18].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[18].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[18].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh.vertices[19].position = { 0.5f,  0.5f, -0.5f, 1.0f }; boxMesh.vertices[19].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[19].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh.vertices[20].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh.vertices[20].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[20].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh.vertices[21].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[21].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[21].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh.vertices[22].position = { 0.5f,  0.5f,  0.5f, 1.0f }; boxMesh.vertices[22].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[22].normal = { 1.0f, 0.0f, 0.0f };
	boxMesh.vertices[23].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[23].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[23].normal = { 1.0f, 0.0f, 0.0f };

	// Top face (Y = 0.5)
	boxMesh.vertices[24].position = { -0.5f, 0.5f, -0.5f, 1.0f }; boxMesh.vertices[24].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[24].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh.vertices[25].position = { -0.5f, 0.5f,  0.5f, 1.0f }; boxMesh.vertices[25].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[25].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh.vertices[26].position = { 0.5f, 0.5f,  0.5f, 1.0f }; boxMesh.vertices[26].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[26].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh.vertices[27].position = { -0.5f, 0.5f, -0.5f, 1.0f }; boxMesh.vertices[27].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[27].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh.vertices[28].position = { 0.5f, 0.5f,  0.5f, 1.0f }; boxMesh.vertices[28].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[28].normal = { 0.0f, 1.0f, 0.0f };
	boxMesh.vertices[29].position = { 0.5f, 0.5f, -0.5f, 1.0f }; boxMesh.vertices[29].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[29].normal = { 0.0f, 1.0f, 0.0f };

	// Bottom face (Y = -0.5)
	boxMesh.vertices[30].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[30].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[30].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh.vertices[31].position = { -0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[31].texcoord = { 0.0f, 0.0f }; boxMesh.vertices[31].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh.vertices[32].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[32].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[32].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh.vertices[33].position = { -0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[33].texcoord = { 0.0f, 1.0f }; boxMesh.vertices[33].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh.vertices[34].position = { 0.5f, -0.5f, -0.5f, 1.0f }; boxMesh.vertices[34].texcoord = { 1.0f, 0.0f }; boxMesh.vertices[34].normal = { 0.0f, -1.0f, 0.0f };
	boxMesh.vertices[35].position = { 0.5f, -0.5f,  0.5f, 1.0f }; boxMesh.vertices[35].texcoord = { 1.0f, 1.0f }; boxMesh.vertices[35].normal = { 0.0f, -1.0f, 0.0f };

	// 面を反転させるかどうかの処理
	if (invertFace) {
		// 法線を反転させる
		for (size_t i = 0; i < boxMesh.vertices.size(); ++i) {
			boxMesh.vertices[i].normal = -boxMesh.vertices[i].normal;
		}
		// 頂点の順序を反転させる
		for (size_t i = 0; i < boxMesh.vertices.size(); i += 3) {
			std::swap(boxMesh.vertices[i], boxMesh.vertices[i + 2]);
		}
	}
	
	return boxMesh;
}

QFE::MeshData QFE::PRIMITIVE::CreateRing(float innerRadius, float outerRadius, uint32_t segments, bool invertFace) {
	MeshData ringMesh(segments * 6);
	for (uint32_t i = 0; i < segments * 6; ++i) {
		ringMesh.vertices.push_back(VertexData());
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
		ringMesh.vertices[i * 6 + 0] = iCurr;
		ringMesh.vertices[i * 6 + 1] = oCurr;
		ringMesh.vertices[i * 6 + 2] = iNext;

		// 三角形2 (内側次、外側現在、外側次)
		ringMesh.vertices[i * 6 + 3] = iNext;
		ringMesh.vertices[i * 6 + 4] = oCurr;
		ringMesh.vertices[i * 6 + 5] = oNext;
	}

	// 面を反転させるかどうかの処理
	if (invertFace) {
		// 法線を反転させる
		for (size_t i = 0; i < ringMesh.vertices.size(); ++i) {
			ringMesh.vertices[i].normal = -ringMesh.vertices[i].normal;
		}
		// 頂点の順序を反転させる
		for (size_t i = 0; i < ringMesh.vertices.size(); i += 3) {
			std::swap(ringMesh.vertices[i], ringMesh.vertices[i + 2]);
		}
	}

	return ringMesh;
}

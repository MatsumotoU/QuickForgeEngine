#include "engine/include/assets/3DModel/PrimitiveVertices.h"

QFE::MeshData QFE::PRIMITIVE::CreateBox(bool invertFace)
{
	MeshData boxMesh(36);
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

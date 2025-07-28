#pragma once
#include <vector>
#include "../Math/VerTexData.h"
#include "../Math/MaterialData.h"

enum CoordinateSystem
{
	COORDINATESYSTEM_HAND_RIGHT,
	COORDINATESYSTEM_HAND_LEFT,
};

struct MeshData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

struct ModelData {
	std::vector<MeshData> meshes;
};

// Modelを使う時のコード
	/*Model model;
	model.Initialize(dxCommon, textureManager, pso);
	model.LoadModel("Resources", "skyDome.obj", COORDINATESYSTEM_HAND_RIGHT);*/

// モデルの描画
	//model.Draw(transform, &debugCamera.camera_);
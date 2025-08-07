#include "LoadModelForEntities.h"
#include "Base/EngineCore.h"
#include "Object/Entity/EntityManager.h"

#include "Object/System/Model/BaseSys/AssimpModelLoader.h"
#include "Object/System/Model/BaseSys/ModelMaterialCreater.h"
#include "Object/System/Model/BaseSys/ModelMeshCreater.h"
#include "Object/System/Model/BaseSys/CreateTransformationComponent.h"

void LoadModelForEntities::Load(EngineCore* engineCore, EntityManager& entityManager, uint32_t entityNum, const std::string& modelFileName) {
	if (!engineCore || modelFileName.empty()) {
		return; // エンジンコアまたはモデルファイル名が無効な場合は何もしない
	}
	ModelData modelData;
	// Assimpを使用してモデルデータをロード
	AssimpModelLoader modelLoader;
	modelLoader.LoadModelData(
		engineCore->GetDirectoryManager().GetDirectoryPath(DirectoryManager::ModelDirectory),
		modelFileName, modelData);
	
	// トランスフォームコンポーネントの作成
	TransformComponent transformComponent;
	CreateTransformationComponent createTransform;
	createTransform.Create(engineCore, transformComponent);
	// エンティティにトランスフォームコンポーネントを追加
	entityManager.EmplaceComponent<TransformComponent>(entityNum, std::move(transformComponent));

	// メッシュとマテリアルのコンポーネントを作成
	for (int i = 0; i < modelData.meshes.size(); i++) {
		// メッシュコンポーネント
		MeshComponent mesh;
		ModelMeshCreater meshCreater;
		meshCreater.CreateResourcesFromModelData(engineCore, modelData.meshes[i].vertices, mesh);
		entityManager.EmplaceComponent<MeshComponent>(entityNum, std::move(mesh));

		// マテリアルコンポーネント
		MaterialComponent material;
		ModelMaterialCreater materialCreater;
		ModelMaterialData modelMaterialData;
		materialCreater.CreateMaterialComponent(engineCore, modelData.meshes[i].material, material);
		entityManager.EmplaceComponent<MaterialComponent>(entityNum, std::move(material));
	}
}

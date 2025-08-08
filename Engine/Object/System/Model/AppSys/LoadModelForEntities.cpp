#include "LoadModelForEntities.h"
#include "Base/EngineCore.h"
#include "Object/Entity/EntityManager.h"

#include "Object/Component/Data/ParentComponent.h"
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
	modelLoader.LoadModelData(engineCore,modelFileName, modelData);
	
	// メッシュとマテリアルのコンポーネントを作成
	for (int i = 0; i < modelData.meshes.size(); i++) {
		entityManager.CreateEntity();

		// トランスフォームコンポーネントの作成
		TransformComponent transformComponent;
		CreateTransformationComponent createTransform;
		createTransform.Create(engineCore, transformComponent);
		// エンティティにトランスフォームコンポーネントを追加
		entityManager.EmplaceComponent<TransformComponent>(entityNum + i, std::move(transformComponent));

		// メッシュコンポーネント
		MeshComponent mesh;
		ModelMeshCreater meshCreater;
		meshCreater.CreateResourcesFromModelData(engineCore, modelData.meshes[i].vertices, mesh);
		entityManager.EmplaceComponent<MeshComponent>(entityNum + i, std::move(mesh));

		// マテリアルコンポーネント
		MaterialComponent material;
		ModelMaterialCreater materialCreater;
		ModelMaterialData modelMaterialData;
		materialCreater.CreateMaterialComponent(engineCore, modelData.meshes[i].material, material);
		entityManager.EmplaceComponent<MaterialComponent>(entityNum + i, std::move(material));

		// 親コンポーネントの追加（必要に応じて）
		if (i > 0) {
			ParentComponent parentComponent;
			parentComponent.parentEntityId = entityNum; // 最初のエンティティを親に設定
			entityManager.EmplaceComponent<ParentComponent>(entityNum + i, std::move(parentComponent));
		}
	}
}

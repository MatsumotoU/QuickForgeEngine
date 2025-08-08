#include "EntitySerializer.h"
#include "Object/Entity/EntityManager.h"

#include "Object/Entity/EntityManager.h"
#include "Object/System/Model/BaseSys/MaterialSerializer.h"
#include "Object/System/Model/BaseSys/MeshSerializer.h"
#include "Object/System/Model/BaseSys/TransformSerializer.h"

nlohmann::json EntitySerializer::Serialize(const EntityManager& entityManager) {
    nlohmann::json jEntities = nlohmann::json::array();

    // EntityIdの範囲を取得
    uint32_t maxId = entityManager.GetNextEntityId();
    for (uint32_t id = 0; id < maxId; ++id) {
        nlohmann::json jEntity;
        jEntity["id"] = id;

        // MaterialComponentが存在する場合
        if (entityManager.HasComponent<MaterialComponent>(id)) {
            const auto& material = entityManager.GetComponent<MaterialComponent>(id);
            jEntity["MaterialComponent"] = MaterialSerializer::Serialize(material);
        }

		// TransformComponentが存在する場合
		if (entityManager.HasComponent<TransformComponent>(id)) {
			const auto& transform = entityManager.GetComponent<TransformComponent>(id);
			jEntity["TransformComponent"] = TransformSerializer::Serialize(transform);
		}

		// MeshComponentが存在する場合
		if (entityManager.HasComponent<MeshComponent>(id)) {
			const auto& mesh = entityManager.GetComponent<MeshComponent>(id);
			jEntity["MeshComponent"] = MeshSerializer::Serialize(mesh);
		}

        // 何かしらのコンポーネントがあれば追加
        if (jEntity.size() > 1) { // "id"以外に何かあれば
            jEntities.push_back(jEntity);
        }
    }
    return jEntities;
}

void EntitySerializer::Deserialize(EngineCore* engineCore, EntityManager& entityManager, const nlohmann::json& j) {
	for (const auto& jEntity : j) {
		uint32_t id = jEntity["id"].get<uint32_t>();
		entityManager.CreateEntity(); // Entityを作成
		// MaterialComponentのデシリアライズ
		if (jEntity.contains("MaterialComponent")) {
			MaterialComponent materialComponent;
			MaterialSerializer::Deserialize(engineCore, materialComponent, jEntity["MaterialComponent"]);
			entityManager.EmplaceComponent<MaterialComponent>(id, std::move(materialComponent));
		}
		// TransformComponentのデシリアライズ
		if (jEntity.contains("TransformComponent")) {
			TransformComponent transformComponent;
			TransformSerializer::Deserialize(engineCore, transformComponent, jEntity["TransformComponent"]);
			entityManager.EmplaceComponent<TransformComponent>(id, std::move(transformComponent));
		}
		// MeshComponentのデシリアライズ
		if (jEntity.contains("MeshComponent")) {
			MeshComponent meshComponent;
			MeshSerializer::Deserialize(engineCore, meshComponent, jEntity["MeshComponent"]);
			entityManager.EmplaceComponent<MeshComponent>(id, std::move(meshComponent));
		}
	}
}

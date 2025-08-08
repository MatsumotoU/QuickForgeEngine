#include "PearentEntityMove.h"
#include "Object/Entity/EntityManager.h"
#include "Object/Component/Data/ParentComponent.h"
#include "Object/Component/Data/TransformComponent.h"

void ParentEntityMove::Update(EntityManager& entityManager) {
	if (!entityManager.HasComponentStrage<ParentComponent>()) {
		return; // ParentComponentが存在しない場合は何もしない
	}

    const ComponentStrage<ParentComponent>& strage = entityManager.GetComponentStrage<ParentComponent>();
    for (const auto& pair : strage) {
        uint32_t entityId = pair.first;
        const ParentComponent& parent = pair.second;
        
		if (entityManager.HasComponent<TransformComponent>(entityId)) {
			TransformComponent& transform = entityManager.GetComponent<TransformComponent>(entityId);
			if (entityManager.HasComponent<TransformComponent>(parent.parentEntityId)) {
				const TransformComponent& parentTransform = entityManager.GetComponent<TransformComponent>(parent.parentEntityId);

				transform.transformationBuffer_.GetData()->World = Matrix4x4::Multiply(
					transform.transformationBuffer_.GetData()->World,Matrix4x4::MakeAffineMatrix(parentTransform.transform_));
			}
		}
    }
}

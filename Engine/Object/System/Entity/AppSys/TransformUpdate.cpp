#include "TransformUpdate.h"

void TransformUpdate::Update(EntityManager& entityManager) {
	if (!entityManager.HasComponentStrage<TransformComponent>()) {
		return; // TransformComponentが存在しない場合は何もしない
	}

	auto& transformStrage = entityManager.GetComponentStrage<TransformComponent>();
	for (auto& [entityId, transform] : transformStrage) {
		TransformComponent& transformComponent = entityManager.GetComponent<TransformComponent>(entityId);

		transformComponent.transformationBuffer_.GetData()->World = Matrix4x4::MakeAffineMatrix(transform.transform_);
	}
}

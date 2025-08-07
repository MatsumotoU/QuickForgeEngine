#include "TransformUpdate.h"

void TransformUpdate::Update(EntityManager& entityManager) {
	auto& transformStrage = entityManager.GetComponentStrage<TransformComponent>();
	for (auto& [entityId, transform] : transformStrage) {
		entityManager.GetComponent<TransformComponent>(entityId).transformationBuffer_.GetData()->World =
			Matrix4x4::MakeAffineMatrix(transform.transform_);
	}
}

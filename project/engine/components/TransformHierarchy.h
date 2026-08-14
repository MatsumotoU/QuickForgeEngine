#pragma once

#include "ParentComponent.h"
#include "ObjectInfoComponent.h"
#include "TransformComponent.h"
#include "design-patterns/EntityManager.h"

#include <unordered_set>

namespace QFE::SCENE {
	/// @brief 永続UUIDから現在の実行時Entity IDを解決する。
	[[nodiscard]] inline bool TryGetEntityIdByUuid(
		const EntityManager& entityManager,
		const std::string& uuid,
		uint32_t& entityId) {
		if (uuid.empty()) {
			return false;
		}
		for (const uint32_t candidateId : entityManager.GetActiveEntityIds()) {
			if (entityManager.HasComponent<ObjectInfoComponent>(candidateId) &&
				entityManager.GetComponent<ObjectInfoComponent>(candidateId).uuid == uuid) {
				entityId = candidateId;
				return true;
			}
		}
		return false;
	}

	/// @brief 親をたどり、エンティティのワールド行列を取得する。
	/// @param localOverride 対象エンティティだけに使用する一時的なローカルTransform。
	[[nodiscard]] inline MATH::Matrix4x4 GetWorldMatrix(
		const EntityManager& entityManager,
		uint32_t entityId,
		const MATH::EulerTransform* localOverride = nullptr) {
		MATH::Matrix4x4 worldMatrix = MATH::Matrix4x4::MakeIdentity4x4();
		if (localOverride != nullptr) {
			worldMatrix = MATH::Matrix4x4::MakeAffineMatrix(*localOverride);
		} else if (entityManager.HasComponent<TransformComponent>(entityId)) {
			worldMatrix = MATH::Matrix4x4::MakeAffineMatrix(
				entityManager.GetComponent<TransformComponent>(entityId).transform);
		}

		std::unordered_set<uint32_t> visitedEntityIds{ entityId };
		uint32_t currentEntityId = entityId;
		while (entityManager.HasComponent<ParentComponent>(currentEntityId)) {
			const ParentComponent& parent =
				entityManager.GetComponent<ParentComponent>(currentEntityId);
			uint32_t parentEntityId = 0;
			if (!parent.HasParent() ||
				!TryGetEntityIdByUuid(entityManager, parent.parent.uuid, parentEntityId) ||
				!visitedEntityIds.insert(parentEntityId).second) {
				break;
			}

			currentEntityId = parentEntityId;
			if (entityManager.HasComponent<TransformComponent>(currentEntityId)) {
				const MATH::Matrix4x4 parentLocalMatrix = MATH::Matrix4x4::MakeAffineMatrix(
					entityManager.GetComponent<TransformComponent>(currentEntityId).transform);
				worldMatrix = MATH::Matrix4x4::Multiply(worldMatrix, parentLocalMatrix);
			}
		}
		return worldMatrix;
	}

	/// @brief 子エンティティから見た親のワールド行列を取得する。
	[[nodiscard]] inline MATH::Matrix4x4 GetParentWorldMatrix(
		const EntityManager& entityManager,
		uint32_t childEntityId) {
		if (!entityManager.HasComponent<ParentComponent>(childEntityId)) {
			return MATH::Matrix4x4::MakeIdentity4x4();
		}

		const ParentComponent& parent =
			entityManager.GetComponent<ParentComponent>(childEntityId);
		uint32_t parentEntityId = 0;
		if (!parent.HasParent() ||
			!TryGetEntityIdByUuid(entityManager, parent.parent.uuid, parentEntityId)) {
			return MATH::Matrix4x4::MakeIdentity4x4();
		}
		return GetWorldMatrix(entityManager, parentEntityId);
	}

	/// @brief parentEntityId を設定したとき循環参照になるか判定する。
	[[nodiscard]] inline bool WouldCreateParentCycle(
		const EntityManager& entityManager,
		uint32_t childEntityId,
		uint32_t parentEntityId) {
		if (childEntityId == parentEntityId) {
			return true;
		}

		std::unordered_set<uint32_t> visitedEntityIds;
		uint32_t currentEntityId = parentEntityId;
		while (entityManager.IsActiveEntity(currentEntityId) &&
			visitedEntityIds.insert(currentEntityId).second) {
			if (currentEntityId == childEntityId) {
				return true;
			}
			if (!entityManager.HasComponent<ParentComponent>(currentEntityId)) {
				return false;
			}
			const ParentComponent& parent =
				entityManager.GetComponent<ParentComponent>(currentEntityId);
			if (!parent.HasParent()) {
				return false;
			}
			if (!TryGetEntityIdByUuid(entityManager, parent.parent.uuid, currentEntityId)) {
				return false;
			}
		}
		return false;
	}

	/// @brief 実行中のEntityを指定して親子関係を設定する。保存されるのは親のUUID。
	[[nodiscard]] inline bool SetParent(
		EntityManager& entityManager,
		uint32_t childEntityId,
		uint32_t parentEntityId) {
		if (!entityManager.IsActiveEntity(childEntityId) ||
			!entityManager.IsActiveEntity(parentEntityId) ||
			!entityManager.HasComponent<ObjectInfoComponent>(parentEntityId) ||
			WouldCreateParentCycle(entityManager, childEntityId, parentEntityId)) {
			return false;
		}

		const std::string& parentUuid =
			entityManager.GetComponent<ObjectInfoComponent>(parentEntityId).uuid;
		if (parentUuid.empty()) {
			return false;
		}

		ParentComponent& parent =
			entityManager.AddDefaultComponent<ParentComponent>(childEntityId);
		parent.parent.uuid = parentUuid;
		return true;
	}

	/// @brief 子エンティティの親子関係を解除する。
	inline void ClearParent(EntityManager& entityManager, uint32_t childEntityId) {
		if (entityManager.HasComponent<ParentComponent>(childEntityId)) {
			entityManager.GetComponent<ParentComponent>(childEntityId).ClearParent();
		}
	}
}

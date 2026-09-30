#pragma once
#include "Entity.h"
#include <vector>

namespace QFE::ENTITY {
	/// @brief エンティティプール
	class EntityPool {
	public:
		EntityPool() = default;
		~EntityPool() = default;
		/// @brief エンティティを生成する
		uint32_t CreateEntity();
		/// @brief エンティティを削除する
		void DestroyEntity(uint32_t entityId);
		/// @brief エンティティが有効かどうかを確認する
		bool IsValid(uint32_t entityId) const;

	private:
		uint32_t nextEntityId = 0; ///< 次に生成されるエンティティID
		std::vector<QFE::ENTITY::Entity> entities; ///< エンティティのリスト
	};
}

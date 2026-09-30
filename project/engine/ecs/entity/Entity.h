#pragma once
#include <cstdint>

namespace QFE::ENTITY {

	/// @brief エンティティ
	struct Entity {
		uint32_t id; ///< エンティティID
		uint32_t uuid; ///< エンティティUUID
		bool active; ///< エンティティが有効かどうか
	};
}

#pragma once

#include <string>

namespace QFE {
	/// @brief 起動ごとに変わるEntity IDではなく、永続UUIDでEntityを参照する。
	struct EntityReference {
		std::string uuid;

		[[nodiscard]] bool IsEmpty() const {
			return uuid.empty();
		}

		void Clear() {
			uuid.clear();
		}
	};
}

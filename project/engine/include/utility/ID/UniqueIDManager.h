#pragma once
#include <cstdint>
#include <set>

namespace QFE {
	class ListUniqueIDManager {
	public:
		void Reset();
		void AddUsedID(uint32_t id);
		uint32_t GenerateUniqueID();

	private:
		uint32_t currentID_;
		std::set<uint32_t> usedIDSet_;
	};
}
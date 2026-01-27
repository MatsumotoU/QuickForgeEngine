#include "engine/include/utility/ID/UniqueIDManager.h"
using namespace QFE;
void ListUniqueIDManager::Reset() {
	currentID_ = 1;
	usedIDSet_.clear();
}

void ListUniqueIDManager::AddUsedID(uint32_t id) {
	usedIDSet_.insert(id);
	if (id >= currentID_) {
		currentID_ = id + 1;
	}
}

uint32_t ListUniqueIDManager::GenerateUniqueID() {
	// currentID_縺蛍sedIDSet_縺ｫ蟄伜惠縺励↑縺・∪縺ｧ繧､繝ｳ繧ｯ繝ｪ繝｡繝ｳ繝・
	while (usedIDSet_.find(currentID_) != usedIDSet_.end()){
		currentID_++;
	}

	usedIDSet_.insert(currentID_);
	return currentID_++;
}

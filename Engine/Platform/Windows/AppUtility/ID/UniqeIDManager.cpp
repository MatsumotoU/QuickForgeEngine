#include "UniqeIDManager.h"

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
	// currentID_がusedIDSet_に存在しないまでインクリメント
	while (usedIDSet_.find(currentID_) != usedIDSet_.end()){
		currentID_++;
	}

	usedIDSet_.insert(currentID_);
	return currentID_++;
}

#include "DynamicComponent.h"

#include <stdexcept>

QFE::DynamicComponentStorage::DynamicComponentStorage(const DynamicComponentDescriptor& descriptor)
	: descriptor_(descriptor),
	  stableName_(descriptor.stableName ? descriptor.stableName : ""),
	  displayName_(descriptor.displayName ? descriptor.displayName : stableName_) {
	if (descriptor_.apiVersion != 1 || stableName_.empty() ||
		descriptor_.create == nullptr || descriptor_.destroy == nullptr ||
		descriptor_.reflect == nullptr) {
		throw std::invalid_argument("Invalid dynamic component descriptor");
	}
}

QFE::DynamicComponentStorage::~DynamicComponentStorage() {
	Clear();
}

void QFE::DynamicComponentStorage::AddDefaultComponent(uint32_t entityId) {
	RemoveComponent(entityId);
	void* instance = descriptor_.create();
	if (instance == nullptr) {
		throw std::runtime_error("Dynamic component creation failed: " + stableName_);
	}
	components_.emplace(entityId, instance);
}

void QFE::DynamicComponentStorage::RemoveComponent(uint32_t id) {
	auto it = components_.find(id);
	if (it == components_.end()) {
		return;
	}
	descriptor_.destroy(it->second);
	components_.erase(it);
}

bool QFE::DynamicComponentStorage::HasComponent(uint32_t id) const {
	return components_.contains(id);
}

void QFE::DynamicComponentStorage::Clear() {
	for (auto& [entityId, component] : components_) {
		(void)entityId;
		descriptor_.destroy(component);
	}
	components_.clear();
}

void QFE::DynamicComponentStorage::ReflectComponent(uint32_t id, Archive& ar) {
	if (void* component = GetRawPtr(id)) {
		descriptor_.reflect(component, ar);
	}
}

std::string QFE::DynamicComponentStorage::GetStorageTypeName() const {
	return stableName_;
}

void* QFE::DynamicComponentStorage::GetRawPtr(uint32_t id) {
	auto it = components_.find(id);
	return it == components_.end() ? nullptr : it->second;
}

void QFE::DynamicComponentStorage::Update(
	uint32_t id, float deltaTime, IEntityManager* entityManager) {
	if (descriptor_.update != nullptr) {
		if (void* component = GetRawPtr(id)) {
			descriptor_.update(id, deltaTime, entityManager, component);
		}
	}
}

const std::string& QFE::DynamicComponentStorage::GetStableName() const {
	return stableName_;
}

const std::string& QFE::DynamicComponentStorage::GetDisplayName() const {
	return displayName_;
}

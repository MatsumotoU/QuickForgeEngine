#include "LockOn.h"

void LockOn::Initialize() {
	reticlePosition_ = { 0.0f,0.0f,0.0f };
	targetPostions_.clear();
}

void LockOn::ResetTargets() {
	targetPostions_.clear();
}

void LockOn::SetReticlePosition(const Vector3& reticle) {
	reticlePosition_ = reticle;
}

void LockOn::AddTargetPosition(const Vector3& target) {
	targetPostions_.push_back(target);
}

Vector3 LockOn::GetLockPosition(Camera* camera) {
	if (targetPostions_.size() <= 0) {
		return Vector3::Zero();
	}

	Vector3 screenReticle = camera->GetWorldToScreenPos(reticlePosition_);
	std::vector<Vector3> screen;
	screen.clear();
	for (Vector3& target : targetPostions_) {
		screen.push_back(camera->GetWorldToScreenPos(target));
	}

	Vector3 result = screen[0];
	float min = (screen[0] - screenReticle).Length();
	for (Vector3& target : screen) {
		if (min >= (target - screenReticle).Length()) {
			min = (target - screenReticle).Length();
			result = target;
		}
	}
	return camera->GetScreenToWorldPos(result);
}

Vector3 LockOn::GetNearestTargetPosition(Camera* camera, int targetNum) {
	if (targetPostions_.empty() || targetNum < 0 || targetNum >= static_cast<int>(targetPostions_.size())) {
		return Vector3::Zero();
	}

	Vector3 screenReticle = camera->GetWorldToScreenPos(reticlePosition_);

	// 距離とインデックスのペアを作成
	std::vector<std::pair<float, int>> distances;
	for (size_t i = 0; i < targetPostions_.size(); ++i) {
		Vector3 screenTarget = camera->GetWorldToScreenPos(targetPostions_[i]);
		float dist = (screenTarget - screenReticle).Length();
		distances.emplace_back(dist, static_cast<int>(i));
	}

	// 距離でソート
	std::sort(distances.begin(), distances.end(),
		[](const std::pair<float, int>& a, const std::pair<float, int>& b) {
			return a.first < b.first;
		});

	int idx = distances[targetNum].second;
	return targetPostions_[idx];
}

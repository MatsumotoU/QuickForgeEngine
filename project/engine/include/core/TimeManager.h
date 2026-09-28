#pragma once

#include "engine/include/utility/DesignPatterns/Singleton.h"

#include <cstddef>
#include <queue>

class TimeManager final : public Singleton<TimeManager> {
	friend class Singleton<TimeManager>;

public:
	void Initialize();
	void Update(float unscaledDeltaTime);
	void StartHitStop(float duration = 0.1f);

	float GetDeltaTime() const { return deltaTime_; }
	float GetUnscaledDeltaTime() const { return unscaledDeltaTime_; }

private:
	TimeManager() = default;
	~TimeManager() = default;

	static constexpr std::size_t kDeltaTimeSampleCount = 512;
	std::queue<float> deltaTimeSamples_;
	float unscaledDeltaTime_ = 0.0f;
	float deltaTime_ = 0.0f;
	float hitStopRemaining_ = 0.0f;
};

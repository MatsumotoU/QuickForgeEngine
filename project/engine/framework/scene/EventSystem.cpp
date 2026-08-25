#include "EventSystem.h"
#include "ComponentParameterSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/EventComponent.h"
#include "components/TransformHierarchy.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string_view>

namespace {
	using Json = nlohmann::json;

	Json Interpolate(const Json& from, const Json& to, float amount, bool step) {
		if (step || amount <= 0.0f) return from;
		if (amount >= 1.0f) return to;
		if (from.is_number() && to.is_number()) {
			return from.get<double>() + (to.get<double>() - from.get<double>()) * amount;
		}
		if (from.is_array() && to.is_array() && from.size() == to.size()) {
			Json result = Json::array();
			for (size_t index = 0; index < from.size(); ++index) {
				result.push_back(Interpolate(from[index], to[index], amount, false));
			}
			return result;
		}
		if (from.is_object() && to.is_object()) {
			Json result = from;
			for (auto iterator = result.begin(); iterator != result.end(); ++iterator) {
				if (to.contains(iterator.key())) {
					iterator.value() = Interpolate(iterator.value(), to[iterator.key()], amount, false);
				}
			}
			return result;
		}
		return from;
	}

	bool EvaluateTrack(const Json& track, float time, Json& output) {
		if (!track.is_object() || !track.contains("keyframes") || !track["keyframes"].is_array()) {
			return false;
		}
		const Json* previous = nullptr;
		const Json* next = nullptr;
		float previousTime = -std::numeric_limits<float>::infinity();
		float nextTime = std::numeric_limits<float>::infinity();
		for (const Json& keyframe : track["keyframes"]) {
			if (!keyframe.is_object() || !keyframe.contains("time") || !keyframe.contains("value") ||
				!keyframe["time"].is_number()) continue;
			const float keyTime = keyframe["time"].get<float>();
			if (keyTime <= time && keyTime >= previousTime) {
				previous = &keyframe;
				previousTime = keyTime;
			}
			if (keyTime >= time && keyTime <= nextTime) {
				next = &keyframe;
				nextTime = keyTime;
			}
		}
		if (previous == nullptr) {
			previous = next;
			previousTime = nextTime;
		}
		if (next == nullptr) {
			next = previous;
			nextTime = previousTime;
		}
		if (previous == nullptr || next == nullptr) return false;

		const bool step = track.value("interpolation", std::string("Linear")) == "Step";
		const float span = nextTime - previousTime;
		const float amount = span > 0.0f ? std::clamp((time - previousTime) / span, 0.0f, 1.0f) : 0.0f;
		output = Interpolate((*previous)["value"], (*next)["value"], amount, step);
		return true;
	}

	float GetDuration(const QFE::SCENE::EventComponent& eventComponent) {
		float duration = 0.0f;
		if (!eventComponent.tracks.is_array()) return duration;
		for (const Json& track : eventComponent.tracks) {
			if (!track.is_object() || !track.contains("keyframes") || !track["keyframes"].is_array()) continue;
			for (const Json& keyframe : track["keyframes"]) {
				if (keyframe.is_object() && keyframe.contains("time") && keyframe["time"].is_number()) {
					duration = (std::max)(duration, keyframe["time"].get<float>());
				}
			}
		}
		return duration;
	}

	void ApplyEvent(QFE::EntityManager& entityManager, uint32_t eventEntityId,
		const QFE::SCENE::EventComponent& eventComponent) {
		if (!eventComponent.tracks.is_array()) return;
		for (const Json& track : eventComponent.tracks) {
			if (!track.is_object()) continue;
			const std::string componentName = track.value("component", std::string{});
			const std::string propertyPath = track.value("property", std::string{});
			if (componentName.empty() || propertyPath.empty()) continue;

			Json value;
			if (!EvaluateTrack(track, eventComponent.currentTime, value)) continue;

			QFE::FRAMEWORK::ComponentParameterRequest request;
			request.componentName = componentName;
			request.propertyPath = propertyPath;
			request.operation = QFE::FRAMEWORK::ParameterOperation::Set;
			request.value = std::move(value);
			const std::string targetUuid = track.value("targetUuid", std::string{});
			if (!targetUuid.empty()) {
				request.target = QFE::FRAMEWORK::ParameterTarget::EntityReference;
				request.entity.uuid = targetUuid;
			}
			QFE::FRAMEWORK::ApplyComponentParameter(
				entityManager, eventEntityId, UINT32_MAX, request);
		}
	}
}

bool QFE::FRAMEWORK::PlayEvent(EntityManager& entityManager, uint32_t eventEntityId, bool restart) {
	auto* eventComponent = entityManager.GetComponentPtr<SCENE::EventComponent>(eventEntityId);
	if (eventComponent == nullptr) return false;
	eventComponent->requestFlags |= restart
		? SCENE::EventRequestRestart
		: SCENE::EventRequestPlay;
	return true;
}

bool QFE::FRAMEWORK::StopEvent(EntityManager& entityManager, uint32_t eventEntityId, bool resetTime) {
	auto* eventComponent = entityManager.GetComponentPtr<SCENE::EventComponent>(eventEntityId);
	if (eventComponent == nullptr) return false;
	eventComponent->requestFlags |= SCENE::EventRequestStop;
	if (resetTime) eventComponent->currentTime = 0.0f;
	return true;
}

bool QFE::FRAMEWORK::IsEventPlaying(const EntityManager& entityManager, uint32_t eventEntityId) {
	const auto* eventComponent = entityManager.GetComponentPtr<SCENE::EventComponent>(eventEntityId);
	return eventComponent != nullptr && eventComponent->playing;
}

void QFE::FRAMEWORK::UpdateEventComponents(EntityManager& entityManager, float deltaTime) {
	entityManager.Each<SCENE::EventComponent>(
		[&](uint32_t entityId, SCENE::EventComponent& eventComponent) {
			if (!eventComponent.initialized) {
				eventComponent.initialized = true;
				if (eventComponent.playOnStart) {
					eventComponent.playing = true;
					eventComponent.currentTime = 0.0f;
				}
			}

			const uint32_t requests = eventComponent.requestFlags;
			eventComponent.requestFlags = SCENE::EventRequestNone;
			if ((requests & SCENE::EventRequestRestart) != 0) {
				eventComponent.currentTime = 0.0f;
				eventComponent.playing = true;
			} else {
				if ((requests & SCENE::EventRequestToggle) != 0) {
					eventComponent.playing = !eventComponent.playing;
				}
				if ((requests & SCENE::EventRequestPlay) != 0) {
					eventComponent.playing = true;
				}
				if ((requests & SCENE::EventRequestStop) != 0) {
					eventComponent.playing = false;
				}
			}
			if (!eventComponent.playing) return;

			const float duration = GetDuration(eventComponent);
			eventComponent.currentTime += (std::max)(0.0f, deltaTime) *
				(std::max)(0.0f, eventComponent.playbackSpeed);
			if (duration > 0.0f && eventComponent.currentTime > duration) {
				if (eventComponent.loop) {
					eventComponent.currentTime = std::fmod(eventComponent.currentTime, duration);
				} else {
					eventComponent.currentTime = duration;
					eventComponent.playing = false;
				}
			}
			ApplyEvent(entityManager, entityId, eventComponent);
		});
}

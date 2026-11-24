#include "framework/scene/CollisionTriggerSystem.h"

#include "ecs/entity/EntityManager.h"
#include "components/CollisionTriggerComponent.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/EventSystem.h"

namespace {
	using Json = nlohmann::json;

	bool TryParseAction(const Json& action,
		QFE::FRAMEWORK::ComponentParameterRequest& request) {
		if (!action.is_object()) return false;
		request.target = QFE::FRAMEWORK::ParameterTargetFromString(
			action.value("target", std::string("Self")));
		request.entity.uuid = action.value("targetUuid", std::string{});
		request.componentName = action.value("component", std::string{});
		request.propertyPath = action.value("property", std::string{});
		request.operation = QFE::FRAMEWORK::ParameterOperationFromString(
			action.value("operation", std::string("Set")));
		request.value = action.contains("value") ? action["value"] : Json{};
		return !request.componentName.empty() && !request.propertyPath.empty();
	}

	void QueueActions(const QFE::SCENE::CollisionTriggerComponent& trigger,
		const char* timing, uint32_t selfEntityId, uint32_t otherEntityId,
		std::vector<QFE::FRAMEWORK::CollisionTriggerCommand>& commandBuffer) {
		if (!trigger.actions.is_array()) return;
		for (const Json& action : trigger.actions) {
			if (!action.is_object() || action.value("timing", std::string("Enter")) != timing) continue;
			commandBuffer.push_back({ selfEntityId, otherEntityId, action });
		}
	}
}

void QFE::FRAMEWORK::BeginCollisionTriggerFrame(EntityManager& entityManager) {
	entityManager.Each<SCENE::CollisionTriggerComponent>(
		[](uint32_t, SCENE::CollisionTriggerComponent& trigger) {
			trigger.currentContacts.clear();
		});
}

void QFE::FRAMEWORK::NotifyCollisionTrigger(
	EntityManager& entityManager, uint32_t colliderEntityId, uint32_t otherEntityId,
	uint32_t otherCollisionMask, std::vector<CollisionTriggerCommand>& commandBuffer) {
	auto* trigger = entityManager.GetComponentPtr<SCENE::CollisionTriggerComponent>(colliderEntityId);
	if (trigger == nullptr ||
		(trigger->acceptedMask != 0 && (trigger->acceptedMask & otherCollisionMask) == 0)) return;

	trigger->currentContacts[otherEntityId] = otherCollisionMask;
	const bool entered = !trigger->previousContacts.contains(otherEntityId);
	if (entered) QueueActions(*trigger, "Enter", colliderEntityId, otherEntityId, commandBuffer);
	QueueActions(*trigger, "Stay", colliderEntityId, otherEntityId, commandBuffer);
}

void QFE::FRAMEWORK::EndCollisionTriggerFrame(
	EntityManager& entityManager, std::vector<CollisionTriggerCommand>& commandBuffer) {
	entityManager.Each<SCENE::CollisionTriggerComponent>(
		[&](uint32_t entityId, SCENE::CollisionTriggerComponent& trigger) {
			for (const auto& [otherEntityId, otherMask] : trigger.previousContacts) {
				(void)otherMask;
				if (!trigger.currentContacts.contains(otherEntityId)) {
					QueueActions(trigger, "Exit", entityId, otherEntityId, commandBuffer);
				}
			}
			trigger.previousContacts = std::move(trigger.currentContacts);
			trigger.currentContacts.clear();
		});
}

void QFE::FRAMEWORK::ApplyCollisionTriggerCommands(EntityManager& entityManager,
	const std::vector<CollisionTriggerCommand>& commands,
	const std::function<void(const std::string&, uint32_t)>& spawn) {
	for (const auto& command : commands) {
		const auto& action = command.action;
		const std::string type = action.value("type", std::string("Parameter"));
		if (type == "Parameter") {
			ComponentParameterRequest request;
			if (TryParseAction(action, request))
				ApplyComponentParameter(entityManager, command.selfEntityId, command.otherEntityId, request);
			continue;
		}
		uint32_t target = command.selfEntityId;
		const auto targetType = ParameterTargetFromString(action.value("target", std::string("Self")));
		if (targetType == ParameterTarget::Other) target = command.otherEntityId;
		else if (targetType == ParameterTarget::EntityReference &&
			!SCENE::TryGetEntityIdByUuid(entityManager, action.value("targetUuid", std::string{}), target)) continue;
		if (!entityManager.IsActiveEntity(target)) continue;
		if (type == "Destroy") entityManager.RemoveEntity(target);
		else if (type == "Spawn") {
			const std::string path = action.value("entityPath", std::string{});
			if (spawn && !path.empty()) spawn(path, target);
		} else if ((type == "PlayTrack" || type == "ToggleTrack") &&
			action.contains("trackIndex") && action["trackIndex"].is_number_integer()) {
			const int64_t index = action["trackIndex"].get<int64_t>();
			if (index < 0) continue;
			if (type == "PlayTrack") PlayEventTrack(entityManager, target, static_cast<size_t>(index));
			else ToggleEventTrack(entityManager, target, static_cast<size_t>(index));
		}
	}
}

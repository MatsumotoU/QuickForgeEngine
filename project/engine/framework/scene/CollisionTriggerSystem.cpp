#include "CollisionTriggerSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/CollisionTriggerComponent.h"

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
		std::vector<QFE::FRAMEWORK::ComponentParameterCommand>& commandBuffer) {
		if (!trigger.actions.is_array()) return;
		for (const Json& action : trigger.actions) {
			if (!action.is_object() || action.value("timing", std::string("Enter")) != timing) continue;
			QFE::FRAMEWORK::ComponentParameterCommand command;
			command.selfEntityId = selfEntityId;
			command.otherEntityId = otherEntityId;
			if (!TryParseAction(action, command.request)) continue;
			commandBuffer.push_back(std::move(command));
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
	uint32_t otherCollisionMask, std::vector<ComponentParameterCommand>& commandBuffer) {
	auto* trigger = entityManager.GetComponentPtr<SCENE::CollisionTriggerComponent>(colliderEntityId);
	if (trigger == nullptr ||
		(trigger->acceptedMask != 0 && (trigger->acceptedMask & otherCollisionMask) == 0)) return;

	trigger->currentContacts[otherEntityId] = otherCollisionMask;
	const bool entered = !trigger->previousContacts.contains(otherEntityId);
	if (entered) QueueActions(*trigger, "Enter", colliderEntityId, otherEntityId, commandBuffer);
	QueueActions(*trigger, "Stay", colliderEntityId, otherEntityId, commandBuffer);
}

void QFE::FRAMEWORK::EndCollisionTriggerFrame(
	EntityManager& entityManager, std::vector<ComponentParameterCommand>& commandBuffer) {
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

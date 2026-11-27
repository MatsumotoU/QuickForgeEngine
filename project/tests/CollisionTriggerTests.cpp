#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/EventComponent.h"
#include "components/CollisionTriggerComponent.h"
#include "components/ObjectInfoComponent.h"
#include "ecs/component/JsonArchive.h"
#include "game/shooting/components/StatusComponent.h"
#include "framework/scene/EventSystem.h"
#include "framework/scene/CollisionTriggerSystem.h"
#include <cassert>
#include <memory>
#include <string>

using namespace QFE;
using namespace QFE::SCENE;
using namespace QFE::FRAMEWORK;

int main() {
    auto registry = std::make_shared<StaticComponentRegistry>();
    registry->RegisterComponent<TransformComponent>("TransformComponent");
    registry->RegisterComponent<EventComponent>("EventComponent");
    registry->RegisterComponent<CollisionTriggerComponent>("CollisionTriggerComponent");
    registry->RegisterComponent<ObjectInfoComponent>("ObjectInfoComponent");
    registry->RegisterComponent<QFE::STG::StatusComponent>("StatusComponent");
    registry->Freeze();
    EntityManager entities(registry);
    const auto self = entities.CreateEntity();
    const auto other = entities.CreateEntity();
    auto& transform = entities.AddDefaultComponent<TransformComponent>(self).transform;
    auto& event = entities.AddDefaultComponent<EventComponent>(self);
    event.tracks = nlohmann::json::array({
        {{"component", "TransformComponent"}, {"property", "transform.translate.x"},
         {"keyframes", {{{"time", 0}, {"value", 0}}, {{"time", 1}, {"value", 10}}}}},
        {{"component", "TransformComponent"}, {"property", "transform.translate.y"},
         {"keyframes", {{{"time", 0}, {"value", 0}}, {{"time", 1}, {"value", 20}}}}}
    });
    auto& trigger = entities.AddDefaultComponent<CollisionTriggerComponent>(self);
    trigger.actions = nlohmann::json::array({
        {{"timing", "Enter"}, {"type", "PlayTrack"}, {"trackIndex", 1}},
        {{"timing", "Exit"}, {"type", "ToggleTrack"}, {"trackIndex", 1}},
        {{"timing", "Enter"}, {"type", "Spawn"}, {"entityPath", "entities/test.entity"}},
        {{"timing", "Enter"}, {"type", "Destroy"}, {"target", "Other"}}
    });
    std::string spawnedPath; uint32_t spawnTarget = UINT32_MAX;
    const auto spawn = [&](const std::string& path, uint32_t target) { spawnedPath = path; spawnTarget = target; };
    std::vector<CollisionTriggerCommand> commands;
    BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other, 0, commands);
    EndCollisionTriggerFrame(entities, commands);
    assert(commands.size() == 3);
    ApplyCollisionTriggerCommands(entities, commands, spawn);
    assert(spawnedPath == "entities/test.entity" && spawnTarget == self);
    UpdateEventComponents(entities, 0.25f);
    assert(transform.translate.x == 0.0f && transform.translate.y == 5.0f);
    commands.clear(); BeginCollisionTriggerFrame(entities); EndCollisionTriggerFrame(entities, commands);
    assert(commands.size() == 1);
    ApplyCollisionTriggerCommands(entities, commands, spawn);
    UpdateEventComponents(entities, 0.25f);
    assert(transform.translate.y == 5.0f);
    assert(ToggleEventTrack(entities, self, 1));
    UpdateEventComponents(entities, 0.25f);
    assert(transform.translate.y == 10.0f);
    assert(!PlayEventTrack(entities, self, 2));
    entities.EndFrame();
    assert(!entities.IsActiveEntity(other));
    trigger.actions = nlohmann::json::array({
        {{"timing", "Enter"}, {"component", "EventComponent"}, {"property", "requestFlags"},
         {"operation", "SetBits"}, {"value", 8}}
    });
    const auto other2 = entities.CreateEntity();
    commands.clear(); BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other2, 0, commands);
    EndCollisionTriggerFrame(entities, commands);
    ApplyCollisionTriggerCommands(entities, commands, spawn);
    UpdateEventComponents(entities, 0.25f);
    assert(transform.translate.x == 2.5f && transform.translate.y == 5.0f);

    // タグ条件は複数候補の OR で、マスク条件とは両方を満たす必要がある。
    trigger.previousContacts.clear();
    trigger.currentContacts.clear();
    trigger.acceptedTags = {"Player", "Ally"};
    trigger.acceptedMask = 2;
    trigger.actions = nlohmann::json::array({
        {{"timing", "Enter"}}, {{"timing", "Stay"}}, {{"timing", "Exit"}}
    });
    auto& info = entities.AddDefaultComponent<ObjectInfoComponent>(other2);
    info.tags = {"Enemy", "Ally"};
    commands.clear(); BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other2, 1, commands);
    EndCollisionTriggerFrame(entities, commands);
    assert(commands.empty());
    BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other2, 2, commands);
    EndCollisionTriggerFrame(entities, commands);
    assert(commands.size() == 2); // Enter と Stay
    commands.clear(); info.tags = {"Enemy"};
    BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other2, 2, commands);
    EndCollisionTriggerFrame(entities, commands);
    assert(commands.size() == 1 && commands[0].action["timing"] == "Exit");

    // タグのない旧シーンは Default に移行し、未設定のトリガーは従来通り全タグを受け付ける。
    nlohmann::json oldInfo = {{"name", "old"}};
    ObjectInfoComponent loadedInfo;
    JsonArchive oldArchive(oldInfo, true);
    loadedInfo.Reflect(oldArchive);
    assert(loadedInfo.tags == std::vector<std::string>{"Default"});
	trigger.acceptedTags = {"Default"};
	const auto withoutInfo = entities.CreateEntity();
	commands.clear(); BeginCollisionTriggerFrame(entities);
	NotifyCollisionTrigger(entities, self, withoutInfo, 2, commands);
	EndCollisionTriggerFrame(entities, commands);
	assert(commands.size() == 2);
	trigger.previousContacts.clear();
    trigger.acceptedTags.clear();
    commands.clear(); BeginCollisionTriggerFrame(entities);
    NotifyCollisionTrigger(entities, self, other2, 2, commands);
    EndCollisionTriggerFrame(entities, commands);
    assert(commands.size() == 2);

	// 接触相手のステータスを減らし、0 未満では巻き戻らず 0 に留める。
	entities.AddDefaultComponent<QFE::STG::StatusComponent>(other2);
	trigger.previousContacts.clear();
	trigger.acceptedTags.clear();
	trigger.acceptedMask = 0;
	trigger.actions = nlohmann::json::array({
		{{"timing", "Enter"}, {"type", "Parameter"}, {"target", "Other"},
		 {"component", "StatusComponent"}, {"property", "health"},
		 {"operation", "Add"}, {"value", -5}}
	});
	commands.clear(); BeginCollisionTriggerFrame(entities);
	NotifyCollisionTrigger(entities, self, other2, 0, commands);
	EndCollisionTriggerFrame(entities, commands);
	ApplyCollisionTriggerCommands(entities, commands, spawn);
	assert(entities.GetComponent<QFE::STG::StatusComponent>(other2).health == 0);
	ComponentParameterRequest request;
	request.target = ParameterTarget::Other;
	request.componentName = "StatusComponent";
	request.propertyPath = "attackPower";
	request.operation = ParameterOperation::Set;
	request.value = 7;
	assert(ApplyComponentParameter(entities, self, other2, request));
	assert(entities.GetComponent<QFE::STG::StatusComponent>(other2).attackPower == 7);

	// 未知のアクションはゲーム側へ順番を保って渡し、共通アクションは渡さない。
	std::vector<std::string> gameActions;
	const std::vector<CollisionTriggerCommand> gameCommands = {
		{ self, other2, {{"type", "IncreaseHealth"}, {"target", "Other"}} },
		{ self, other2, {{"type", "DecreaseHealth"}, {"target", "Self"}} },
		{ self, other2, {{"type", "Parameter"}, {"target", "Other"},
			{"component", "StatusComponent"}, {"property", "health"}, {"operation", "Set"}, {"value", 2}} }
	};
	ApplyCollisionTriggerCommands(entities, gameCommands, {}, [&](const CollisionTriggerCommand& command) {
		gameActions.push_back(command.action.value("type", std::string{}));
	});
	assert((gameActions == std::vector<std::string>{ "IncreaseHealth", "DecreaseHealth" }));
	assert(entities.GetComponent<QFE::STG::StatusComponent>(other2).health == 2);
}

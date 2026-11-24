#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/EventComponent.h"
#include "components/CollisionTriggerComponent.h"
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
}

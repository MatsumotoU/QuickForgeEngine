#include "ecs/entity/EntityManager.h"
#include "ecs/component/StaticComponentRegistry.h"
#include "components/ModelRenderComponent.h"
#include "components/SkeletonAnimationComponent.h"
#include "components/SpriteRenderComponent.h"
#include "components/ParticleEmitterComponent.h"
#include "components/SkyBoxComponent.h"
#include <iostream>
#include <memory>

int main() {
	auto registry = std::make_shared<QFE::StaticComponentRegistry>();
	registry->RegisterComponent<QFE::SCENE::ModelRenderComponent>("ModelRenderComponent");
	registry->RegisterComponent<QFE::SCENE::SkeletonAnimationComponent>("SkeletonAnimationComponent");
	registry->RegisterComponent<QFE::SCENE::SpriteRenderComponent>("SpriteRenderComponent");
	registry->RegisterComponent<QFE::SCENE::ParticleEmitterComponent>("ParticleEmitterComponent");
	registry->RegisterComponent<QFE::SCENE::SkyBoxComponent>("SkyBoxComponent");
	registry->Freeze();
	QFE::EntityManager manager(registry);
	bool passed = true;
	const auto check = [&passed](bool condition, const char* message) {
		if (!condition) { std::cerr << message << '\n'; passed = false; }
	};

	const auto legacyId = manager.CreateEntity();
	const nlohmann::json legacy = {{"ModelRenderComponent", {
		{"modelName", "models/walk.glb"}, {"glbAnimationName", "Walk"},
		{"playGlbAnimation", false}, {"loopGlbAnimation", false}, {"glbAnimationSpeed", 2.0f}}}};
	manager.DeserializeEntityComponents(legacyId, legacy);
	const auto* animation = manager.GetComponentPtr<QFE::SCENE::SkeletonAnimationComponent>(legacyId);
	check(animation && animation->glbAnimationName == "Walk" && !animation->playGlbAnimation &&
		!animation->loopGlbAnimation && animation->glbAnimationSpeed == 2.0f, "Legacy settings were not migrated");
	const auto saved = manager.SerializeEntityComponents(legacyId);
	check(saved.contains("SkeletonAnimationComponent") &&
		!saved.at("ModelRenderComponent").contains("glbAnimationName"), "Animation was not serialized separately");
	const auto restoredId = manager.CreateEntity();
	manager.DeserializeEntityComponents(restoredId, saved);
	const auto* restored = manager.GetComponentPtr<QFE::SCENE::SkeletonAnimationComponent>(restoredId);
	check(restored && restored->glbAnimationName == "Walk" && restored->glbAnimationSpeed == 2.0f, "New settings did not round-trip");

	const auto noAnimationId = manager.CreateEntity();
	manager.DeserializeEntityComponents(noAnimationId, {{"ModelRenderComponent", {{"modelName", "models/walk.glb"}, {"textureName", "images/icon.png"}}}});
	check(!manager.HasComponent<QFE::SCENE::SkeletonAnimationComponent>(noAnimationId), "Removed component was recreated");
	const auto modelSaved = manager.SerializeEntityComponents(noAnimationId);
	check(modelSaved.at("ModelRenderComponent").value("modelName", "") == "models/walk.glb" &&
		modelSaved.at("ModelRenderComponent").value("textureName", "") == "images/icon.png", "Model paths did not round-trip");
	const auto effectsId = manager.CreateEntity();
	manager.DeserializeEntityComponents(effectsId, {
		{"SpriteRenderComponent", {{"textureName", "images/icon.png"}}},
		{"ParticleEmitterComponent", {{"textureName", "images/fire.png"}}},
		{"SkyBoxComponent", {{"textureName", "skybox/sky.dds"}}}});
	const auto savedEffects = manager.SerializeEntityComponents(effectsId);
	check(savedEffects.at("SpriteRenderComponent").value("textureName", "") == "images/icon.png" &&
		savedEffects.at("ParticleEmitterComponent").value("textureName", "") == "images/fire.png" &&
		savedEffects.at("SkyBoxComponent").value("textureName", "") == "skybox/sky.dds", "Image paths did not round-trip");
	const auto explicitId = manager.CreateEntity();
	nlohmann::json explicitSettings = legacy;
	explicitSettings["SkeletonAnimationComponent"] = {{"glbAnimationName", "Run"}};
	manager.DeserializeEntityComponents(explicitId, explicitSettings);
	check(manager.GetComponent<QFE::SCENE::SkeletonAnimationComponent>(explicitId).glbAnimationName == "Run", "Explicit settings did not take priority");
	return passed ? 0 : 1;
}

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
	manager.DeserializeEntityComponents(noAnimationId, {{"ModelRenderComponent", {{"modelName", "models/walk.glb"}, {"modelAssetId", "asset-1"}}}});
	check(!manager.HasComponent<QFE::SCENE::SkeletonAnimationComponent>(noAnimationId), "Removed component was recreated");
	check(manager.SerializeEntityComponents(noAnimationId).at("ModelRenderComponent").value("modelAssetId", "") == "asset-1", "Model asset ID was not serialized");
	const auto textureModelId = manager.CreateEntity();
	manager.DeserializeEntityComponents(textureModelId, {{"ModelRenderComponent", {
		{"modelName", "models/walk.glb"}, {"modelAssetId", "asset-1"},
		{"textureName", "images/icon.png"}, {"textureAssetId", "image-2"}}}});
	const auto savedTextureModel = manager.SerializeEntityComponents(textureModelId);
	const auto restoredTextureModelId = manager.CreateEntity();
	manager.DeserializeEntityComponents(restoredTextureModelId, savedTextureModel);
	check(manager.GetComponent<QFE::SCENE::ModelRenderComponent>(restoredTextureModelId).textureAssetId == "image-2",
		"Model texture asset ID did not round-trip");
	check(manager.GetComponent<QFE::SCENE::ModelRenderComponent>(legacyId).textureAssetId.empty(),
		"Legacy model unexpectedly gained a texture asset ID during deserialization");
	const auto spriteId = manager.CreateEntity();
	manager.DeserializeEntityComponents(spriteId, {{"SpriteRenderComponent", {{"textureName", "images/icon.png"}, {"textureAssetId", "image-1"}}}});
	check(manager.SerializeEntityComponents(spriteId).at("SpriteRenderComponent").value("textureAssetId", "") == "image-1", "Sprite asset ID was not serialized");
	const auto effectsId = manager.CreateEntity();
	manager.DeserializeEntityComponents(effectsId, {
		{"ParticleEmitterComponent", {{"textureName", "images/fire.png"}, {"textureAssetId", "particle-image"}}},
		{"SkyBoxComponent", {{"textureName", "skybox/sky.dds"}, {"textureAssetId", "sky-image"}}}});
	const auto savedEffects = manager.SerializeEntityComponents(effectsId);
	const auto restoredEffectsId = manager.CreateEntity();
	manager.DeserializeEntityComponents(restoredEffectsId, savedEffects);
	check(manager.GetComponent<QFE::SCENE::ParticleEmitterComponent>(restoredEffectsId).textureAssetId == "particle-image" &&
		manager.GetComponent<QFE::SCENE::SkyBoxComponent>(restoredEffectsId).textureAssetId == "sky-image",
		"Particle or skybox image asset ID did not round-trip");
	const auto legacyEffectsId = manager.CreateEntity();
	manager.DeserializeEntityComponents(legacyEffectsId, {
		{"ParticleEmitterComponent", {{"textureName", "images/old.png"}}},
		{"SkyBoxComponent", {{"textureName", "skybox/old.dds"}}}});
	check(manager.GetComponent<QFE::SCENE::ParticleEmitterComponent>(legacyEffectsId).textureAssetId.empty() &&
		manager.GetComponent<QFE::SCENE::ParticleEmitterComponent>(legacyEffectsId).textureName == "images/old.png" &&
		manager.GetComponent<QFE::SCENE::SkyBoxComponent>(legacyEffectsId).textureAssetId.empty() &&
		manager.GetComponent<QFE::SCENE::SkyBoxComponent>(legacyEffectsId).textureName == "skybox/old.dds",
		"Legacy particle or skybox texture reference was not preserved");
	const auto explicitId = manager.CreateEntity();
	nlohmann::json explicitSettings = legacy;
	explicitSettings["SkeletonAnimationComponent"] = {{"glbAnimationName", "Run"}};
	manager.DeserializeEntityComponents(explicitId, explicitSettings);
	check(manager.GetComponent<QFE::SCENE::SkeletonAnimationComponent>(explicitId).glbAnimationName == "Run", "Explicit settings did not take priority");
	return passed ? 0 : 1;
}

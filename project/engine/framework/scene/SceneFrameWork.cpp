#include "SceneFrameWork.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"

uint32_t QFE::FRAMEWORK::CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name) {
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	uint32_t entity = entityManager.CreateEntity();
	entityManager.EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entity);
	entityManager.EmplaceComponent<QFE::SCENE::TransformComponent>(entity);
	entityManager.GetComponent<QFE::SCENE::ObjectInfoComponent>(entity).name = name;
	return entity;
}

uint32_t QFE::FRAMEWORK::CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor) {
	uint32_t entity = CreateEntity(sceneManager, name);
	EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	entityManager.EmplaceComponent<QFE::SCENE::MaterialComponent>(entity);
	entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entity).albedoColor = albedoColor;
	return entity;
}

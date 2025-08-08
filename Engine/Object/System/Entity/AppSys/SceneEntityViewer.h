#pragma once
#ifdef _DEBUG
#include <cstdint>
class EntityManager;

class SceneEntityViewer {
public:
	// シーン内のエンティティを表示する
	static void DisplayEntities(EntityManager& entityManager);
	// エンティティの詳細情報を表示する
	static void DisplayEntityDetails(EntityManager& entityManager, uint32_t entityId);
};
#endif // _DEBUG
#include "TestGameLogic.h"
#include "components/AllComponent.h"
#include <vector>

namespace {
	std::vector<QFE::SCRIPT::ScriptFunctionInfo> s_manifestList;
}

void Script_PlayerMove(uint32_t entityId, float dt, QFE::EntityManager* entityManager) {
	auto& transformComp = QFE::SCRIPT::GetComponent<QFE::SCENE::TransformComponent>(entityId, entityManager);

	transformComp.transform.translate.x += 1.0f * dt;
}

void Script_EnemyAI(uint32_t entityId, float dt, QFE::EntityManager* entityManager) {
	auto& transformComp = QFE::SCRIPT::GetComponent<QFE::SCENE::TransformComponent>(entityId, entityManager);

	transformComp.transform.translate.x -= 1.0f * dt;
}
extern "C" {
	__declspec(dllexport) size_t GetManifest(QFE::SCRIPT::ScriptFunctionInfo** outArray) {
		// 最初に1回だけリストを構築
		if (s_manifestList.empty()) {
			s_manifestList.push_back({ "Script_PlayerMove", "Player Move", Script_PlayerMove });
			s_manifestList.push_back({ "Script_EnemyAI", "Enemy AI", Script_EnemyAI });
		}

		// 配列の先頭アドレスをEXE側に教えてあげる
		*outArray = s_manifestList.data();

		// 個数を返す
		return s_manifestList.size();
	}
}
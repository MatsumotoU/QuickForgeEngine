#pragma once

#include "EngineDefines.h"
#include "ecs/component/EntityReference.h"

#include <cstdint>
#include <string>
#include <vector>

namespace QFE::STG {
	/// 部屋の扉は閉じた位置に配置し、開くときだけ指定量を加算する。
	struct EncounterRoomDoor {
		QFE::EntityReference entity;
		QFE::MATH::Vector3 openOffset{ 0.0f, 4.0f, 0.0f };
		static std::string GetTypeName() { return "EncounterRoomDoor"; }
		void Reflect(QFE::Archive& ar) { ar.Process("entity", entity); ar.Process("openOffset", openOffset); }
	};

	struct EncounterRoomEnemy {
		QFE::EntityReference entity;
		static std::string GetTypeName() { return "EncounterRoomEnemy"; }
		void Reflect(QFE::Archive& ar) { ar.Process("entity", entity); }
	};

	/// 生成位置は部屋の中心からのワールド座標上の差分。
	struct EncounterRoomSpawn {
		std::string entityPath;
		QFE::MATH::Vector3 offset{ 0.0f, 0.0f, 0.0f };
		static std::string GetTypeName() { return "EncounterRoomSpawn"; }
		void Reflect(QFE::Archive& ar) { ar.Process("entityPath", entityPath); ar.Process("offset", offset); }
	};

	/// 入室後に対象の敵がいなくなるまで扉を閉じる。
	struct EncounterRoomComponent {
		bool enabled = true;
		QFE::EntityReference player;
		QFE::MATH::Vector3 halfExtents{ 5.0f, 5.0f, 5.0f };
		float doorCloseDuration = 0.5f; ///< 閉じるまでの秒数。0 以下なら即座に閉じる。
		std::vector<EncounterRoomDoor> doors;
		std::vector<EncounterRoomEnemy> enemies;
		std::vector<EncounterRoomSpawn> spawns;

		// 実行時状態は保存しない。扉の閉位置は初期配置から取得する。
		bool initialized = false;
		bool started = false;
		bool cleared = false;
		bool spawnFailed = false;
		float doorCloseElapsed = 0.0f;
		std::vector<uint32_t> doorIds;
		std::vector<QFE::MATH::Vector3> closedDoorPositions;
		std::vector<uint32_t> enemyIds;

		QFE_REFLECT_BEGIN(EncounterRoomComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(player)
			QFE_REFLECT_MEMBER(halfExtents)
			QFE_REFLECT_MEMBER(doorCloseDuration)
			QFE_REFLECT_MEMBER(doors)
			QFE_REFLECT_MEMBER(enemies)
			QFE_REFLECT_MEMBER(spawns)
		QFE_REFLECT_END()
	};
}

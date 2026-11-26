#pragma once
#include "EngineDefines.h"
#include "math/vector/Vector3.h"
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

namespace QFE::SCENE {
	/// イベント等から別シーンの追加を要求する。位置はワールド座標。
	struct SceneSpawnComponent {
		// プロジェクトの scenes フォルダーからの相対パスを順番に保持する。
		std::vector<std::string> scenePaths;
		uint32_t selectedSceneIndex = 0;
		bool spawnAll = false;
		QFE::MATH::Vector3 position{ 0.0f, 0.0f, 0.0f };
		bool request = false;
		// true が続く間は一度だけ生成し、false で次の要求を受け付ける。
		bool requestHandled = false;
		struct PendingRequest { uint32_t sceneIndex; bool all; };
		// 当たり判定から同じフレームに複数回要求されても失わない。
		std::vector<PendingRequest> pendingRequests;

		QFE_REFLECT_BEGIN(SceneSpawnComponent)
			// 旧形式の単一パスを一覧の先頭へ移す。新形式がある場合はそちらを優先する。
			if (ar.IsLoading() && !ar.HasField("scenePaths") && ar.HasField("scenePath")) {
				std::string oldPath;
				ar.Process("scenePath", oldPath);
				scenePaths.clear();
				if (!oldPath.empty()) scenePaths.push_back(std::move(oldPath));
			}
			QFE_REFLECT_MEMBER(scenePaths)
			QFE_REFLECT_MEMBER(selectedSceneIndex)
			QFE_REFLECT_MEMBER(spawnAll)
			QFE_REFLECT_MEMBER(position)
			QFE_REFLECT_MEMBER(request)
		QFE_REFLECT_END()
	};
}

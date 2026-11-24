#pragma once

#include <cstdint>
#include <cstddef>

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	/// EventComponent::tracks の各要素:
	/// { "targetUuid":"" (空なら自身), "component":"TransformComponent",
	///   "property":"transform.translate.x", "interpolation":"Linear",
	///   "keyframes":[{"time":0.0,"value":0.0},{"time":1.0,"value":10.0}] }
	/// value は数値だけでなく bool/string/Vector等のJSONオブジェクトにも対応する。
	bool PlayEvent(EntityManager& entityManager, uint32_t eventEntityId, bool restart = true);
	bool StopEvent(EntityManager& entityManager, uint32_t eventEntityId, bool resetTime = false);
	bool IsEventPlaying(const EntityManager& entityManager, uint32_t eventEntityId);
	/// A track started independently owns its playback until the whole event is played again.
	bool PlayEventTrack(EntityManager& entityManager, uint32_t eventEntityId, size_t trackIndex);
	bool ToggleEventTrack(EntityManager& entityManager, uint32_t eventEntityId, size_t trackIndex);

	/// 全EventComponentのタイムラインを進め、対象パラメータへ反映する。
	void UpdateEventComponents(EntityManager& entityManager, float deltaTime);

}

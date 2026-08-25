#pragma once

#include <cstdint>
#include "../../externals/nlohmann/json.hpp"

#include "EngineDefines.h"

namespace QFE::SCENE {
	enum EventRequestFlags : uint32_t {
		EventRequestNone = 0,
		EventRequestPlay = 1u << 0,
		EventRequestStop = 1u << 1,
		EventRequestToggle = 1u << 2,
		EventRequestRestart = 1u << 3
	};

	/// @brief 他のコンポーネントの値をタイムラインで操作するコンポーネント。
	/// tracks の形式は EventSystem.h のドキュメントを参照。
	struct EventComponent {
		bool playOnStart = false;
		bool loop = false;
		float playbackSpeed = 1.0f;
		uint32_t requestFlags = EventRequestNone;
		nlohmann::json tracks = nlohmann::json::array();

		// ランタイム状態（シーンには保存しない）。
		bool playing = false;
		bool initialized = false;
		float currentTime = 0.0f;

		QFE_REFLECT_BEGIN(EventComponent)
			QFE_REFLECT_MEMBER(playOnStart)
			QFE_REFLECT_MEMBER(loop)
			QFE_REFLECT_MEMBER(playbackSpeed)
			QFE_REFLECT_MEMBER(requestFlags)
			QFE_REFLECT_MEMBER(tracks)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(EventComponent)
}

#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include <list>

namespace QFE {
	/// @brief アニメーションコンポーネントのデータ構造体
	class AnimationComponent final : public ComponentData {
	public:
		std::string clipName; ///< アニメーションクリップの名前

		uint32_t clipHandle; ///< アニメーションクリップのハンドル（実際のAnimClipへの参照）
		std::list<uint32_t> playingAnimHandles; ///< 再生中のアニメーションクリップのハンドルのリスト（複数同時再生を想定）

		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;
		std::string GetTypeName() const override { return "AnimationComponent"; }
	};
	
}
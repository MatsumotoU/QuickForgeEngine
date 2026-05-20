#pragma once
#include "string"
#include "AnimClip.h" 
#include <unordered_map>

namespace QFE {
	/// @brief アニメーションクリップをハンドル管理するコンテナクラス
	class AnimationClipContainer final {
	public:
		/// @brief アニメーションクリップコンテナの初期化と終了処理
		void Initialize(size_t containerSize);
		/// @brief アニメーションクリップコンテナの終了処理
		void Finalize();

		// アニメーションクリップを登録
		uint32_t RegisterAnimationClip(AnimClip animClip);
		// アニメーションクリップを取得
		const AnimClip& GetAnimationClip(uint32_t animationId) const;

	private:
		AnimClip dummyClip_;

		std::unordered_map<uint32_t, AnimClip> animationClips_;
		size_t containerSize_;
		uint32_t nextAnimationId_;
	};
}

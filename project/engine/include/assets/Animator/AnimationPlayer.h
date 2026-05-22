#pragma once
#include "engine/include/utility/memory/SparseSets.h"
#include "AnimationPlayClip.h"

namespace QFE {
	/// @brief アニメーションクリップのポインタから再生を管理するクラス
	class AnimationPlayer {
	public:
		void Initialize();
		void Finalize();

		/// @brief アニメーションの再生状態を更新する。deltaTimeは前フレームからの経過時間。
		void Update(float deltaTime);
		/// @brief フレームの終了処理。必要に応じて再生中のアニメーションを停止するなどの処理を行う。
		void FrameEnd();

		/// @brief アニメーションクリップを再生する。返り値は再生中のアニメーションIDで、停止する際に使用する。
		uint32_t PlayAnimation(AnimClip* animClip,float playSpeed =1.0f,float startTime = 0.0f);
		/// @brief 再生中のアニメーションを停止する。引数はPlayAnimationの返り値であるアニメーションID。
		void StopAnimation(uint32_t animationId);

		/// @brief 再生中のアニメーションが存在するかどうかを確認する。引数はPlayAnimationの返り値であるアニメーションID。
		bool HasAnimation(uint32_t animationId) const;
		/// @brief 再生中のアニメーションの情報を取得する。引数はPlayAnimationの返り値であるアニメーションID。存在しない場合はnullptrを返す。
		AnimationPlayClip* GetAnimationPlayClip(uint32_t animationId);
		/// @brief 死んだアニメーションのIDを取得する。これらのアニメーションは次のFrameEndで停止される予定である。
		std::vector<uint32_t> GetDeleteAnimations() const { return deleteAnimations_; }

		/// @brief 現在再生中の全てのアニメーションの情報を取得する。返り値はアニメーションIDとアニメーション情報のペアのリストである。
		std::vector<AnimationPlayClip> GetActiveAnimations() const { return activeAnimations_.Values(); }

	private:
		std::vector<uint32_t> deleteAnimations_;
		SparseSet<AnimationPlayClip> activeAnimations_;
	};
} // namespace QFE

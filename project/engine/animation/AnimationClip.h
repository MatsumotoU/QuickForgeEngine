#pragma once

#include <string>
#include <vector>

#include "math/MathInclude.h"

namespace QFE::ANIMATION {
	struct AnimationKeyFrame {
		float time = 0.0f;
		MATH::EulerTransform transform;
	};

	/// @brief Transformキーフレームを保持し、任意時刻の姿勢を補間するクリップ
	class AnimationClip final {
	public:
		/// @brief クリップの名前を設定する
		void SetName(const std::string& name) { name_ = name; }
		/// @brief クリップの名前を取得する
		const std::string& GetName() const { return name_; }
		/// @brief ループ設定を行う
		void SetLoop(bool loop) { loop_ = loop; }
		/// @brief ループ設定を取得する
		bool IsLoop() const { return loop_; }

		/// @brief キーフレームを追加する
		void AddKeyFrame(const AnimationKeyFrame& keyFrame);
		/// @brief キーフレームを全て削除する
		void ClearKeyFrames();
		/// @brief キーフレームを取得する
		/// @return キーフレームの配列
		const std::vector<AnimationKeyFrame>& GetKeyFrames() const { return keyFrames_; }
		/// @brief クリップの再生時間を取得する
		/// @return 再生時間（秒）
		float GetDuration() const;
		/// @brief 任意時間の姿勢を補間して取得する
		/// @param time 時間（秒）
		/// @return 補間された姿勢
		MATH::EulerTransform Sample(float time) const;

	private:
		std::string name_;
		bool loop_ = false;
		std::vector<AnimationKeyFrame> keyFrames_;
	};
}

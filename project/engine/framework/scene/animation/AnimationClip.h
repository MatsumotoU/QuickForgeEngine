#pragma once

#include <string>
#include <vector>

#include "math/MathInclude.h"

namespace QFE::ANIMATION {
	struct AnimationKeyFrame {
		float time = 0.0f;
		MATH::EulerTransform transform;
	};

	/// @brief Transformキーフレームを保持し、任意時刻の姿勢を補間するクリップ。
	class AnimationClip final {
	public:
		void SetName(const std::string& name) { name_ = name; }
		const std::string& GetName() const { return name_; }
		void SetLoop(bool loop) { loop_ = loop; }
		bool IsLoop() const { return loop_; }

		void AddKeyFrame(const AnimationKeyFrame& keyFrame);
		void ClearKeyFrames();
		const std::vector<AnimationKeyFrame>& GetKeyFrames() const { return keyFrames_; }
		float GetDuration() const;
		MATH::EulerTransform Sample(float time) const;

	private:
		std::string name_;
		bool loop_ = false;
		std::vector<AnimationKeyFrame> keyFrames_;
	};
}

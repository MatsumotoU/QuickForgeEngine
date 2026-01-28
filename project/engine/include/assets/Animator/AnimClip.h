#pragma once
#include <vector>
#include <string>
#include "KeyFrame.h"

namespace QFE {

	class AnimClip final {
	public:
		~AnimClip() = default;
		AnimClip() = delete;
		explicit AnimClip(const std::string& name);
		// コピーコンストラクタとコピー代入を再生成
		AnimClip(const AnimClip&) noexcept = default;
		AnimClip& operator=(const AnimClip&) noexcept = default;

		void SetLoop(bool isLoop);
		bool IsLoop() const;
		void AddKeyFrame(const KeyFrame& keyframe);
		const std::vector<KeyFrame>& GetKeyFrames() const;
		size_t GetKeyFrameCount() const;
		Transform GetTransformAtTime(float time) const;

	private:
		bool isLoop_;
		const std::string name_;
		std::vector<KeyFrame> keyframes_;
	};

}

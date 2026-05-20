#pragma once
#include <vector>
#include <string>
#include "KeyFrame.h"

#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {
	/// @brief アニメーション全体を表すクラス。複数のキーフレームを持ち、時間に応じたTransformを提供する。
	class AnimClip final {
	public:
		explicit AnimClip(size_t initialCapacity = 50);

		/// @brief アニメーションクリップの名前を設定する
		void SetName(const std::string& name);
		/// @brief アニメーションクリップの名前を取得する
		const std::string& GetName() const;

		/// @brief ループするかどうかを設定する
		void SetLoop(bool isLoop);
		/// @brief ループするかどうかを取得する
		bool IsLoop() const;

		/// @brief キーフレームを追加する
		void AddKeyFrame(const KeyFrame& keyframe);

		/// @brief キーフレームのリストを取得する
		std::vector<KeyFrame> GetKeyFrames() const;
		/// @brief キーフレームの数を取得する
		size_t GetKeyFrameCount() const;


		/// @brief 指定した時間におけるTransformを取得する
		Transform GetTransformAtTime(float time) const;
		/// @brief アニメーションクリップの総時間を取得する
		float GetTotalDuration() const;

	private:
		bool isLoop_;
		std::string name_;
		SafeVector<KeyFrame> keyframes_;
	};

}

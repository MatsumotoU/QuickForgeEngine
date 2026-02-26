/**
 * @file MyMath.h
 * @brief 数学ユーティリティ関数の定義
 */

#pragma once
#include "MathInclude.h"
#include "Shapes.h"
#include <random>

namespace QFE {

	/**
	 * @namespace MyMath
	 * @brief 補間、変換、ランダム値生成などの数学関数を提供
	 */
	namespace MyMath {
		/** @brief 線形補間(Linear Interpolation) */
		float Lerp(float a, float b, float t);
		/** @brief 球面線形補間(Spherical Linear Interpolation) */
		float Slerp(float from, float to, float t);
		/** @brief 簡易的なイーズイン処理 */
		void SimpleEaseIn(float* value, float endValue, float transitionSpeed);
		/** @brief 簡易的なイーズイン補間 */
		float SimpleEaseIn(float from, float to, float transitionSpeed);
		/** @brief 度数からラジアンへ変換 */
		float DegreesToRadians(float degrees);

		template<typename T>
		inline T EaseIn(T from, T to, float t) {
			return from + (to - from) * t * t;
		}
		template<typename T>
		inline T EaseOut(T from, T to, float t) {
			return from + (to - from) * (1 - (1 - t) * (1 - t));
		}
		template<typename T>
		inline T EaseInOut(T from, T to, float t) {
			if (t < 0.5f) {
				return EaseIn(from, (from + to) / 2, t * 2);
			} else {
				return EaseOut((from + to) / 2, to, (t - 0.5f) * 2);
			}
		}

		/**
		 * @brief 指定範囲の乱数を生成
		 * @param min 最小値
		 * @param max 最大値
		 * @return 生成された乱数
		 */
		inline float Rand(float min, float max) {
			static std::random_device rd;
			static std::mt19937 mt(rd());
			std::uniform_real_distribution<float> dist(min, max);
			return dist(mt);
		}

		/** @brief 球体に最も近いAABB上の点を取得 */
		Vector3 ClosestPoint(const Sphere& sphere, const AABB& aabb);

		/// @brief 大きな数の素数判定
		bool IsPrime(uint64_t number);
	}

}

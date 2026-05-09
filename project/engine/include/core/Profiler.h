#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include <chrono>
#include <string>
#include <deque>
#include <unordered_map>
#include <source_location>

namespace QFE {
	/**
	 * プロファイラークラス.
	 * コードの実行時間を計測するためのクラスです.
	 */
	class Profiler : public Singleton<Profiler> {
		friend class Singleton<Profiler>;
	public:
		/// @brief 初期化処理.
		void Initialize();
		/// @brief 終了処理.
		void Finalize();
		// / @brief フレーム開始処理.
		void FrameStart();
		/// @brief フレーム終了処理.
		void FrameEnd();

		/// @brief スコーププロファイルをクリアします.
		void ClearScopeProfiles();

		/// @brief スコーププロファイルを記録します.
		void RecordScopeProfile(const std::string& scopeName, const std::chrono::milliseconds& duration);

		/// @brief スコーププロファイルを取得します.
		const std::unordered_map<std::string, std::deque<std::chrono::milliseconds>>& GetScopeProfiles() const;
		/// @brief フレーム時間を取得します.
		std::chrono::milliseconds GetFrameDuration() const;
		/// @brief あるスコープの平均時間を取得します.
		std::chrono::milliseconds GetAverageScopeDuration(const std::string& scopeName) const;

	private:
		~Profiler() override = default;

		std::chrono::high_resolution_clock::time_point frameStartTime_;
		std::chrono::high_resolution_clock::time_point frameEndTime_;
		std::chrono::milliseconds frameDuration_;

		size_t maxProfileEntries_ = 100;// プロファイルの最大エントリー数. 古いエントリーは削除されます.

		// スコーププロファイルのマップ. スコープ名をキーにしてプロファイルを保存します.
		std::unordered_map<std::string, std::deque<std::chrono::milliseconds>> scopeProfiles_;
	};

	/// スコーププロファイルクラス.
	class ScopeProfile {
	public:
		ScopeProfile(const std::string& scopeName) : scopeName_(scopeName) {
			try {
				startTime_ = std::chrono::high_resolution_clock::now();
			}
			catch (const std::exception&) {}
		}
		~ScopeProfile() {
			try {
				auto endTime = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime_);
				Profiler::GetInstance()->RecordScopeProfile(scopeName_, duration);
			}
			catch (const std::exception&) {}
		}

	private:
		std::string scopeName_;
		std::chrono::high_resolution_clock::time_point startTime_;
	};
}



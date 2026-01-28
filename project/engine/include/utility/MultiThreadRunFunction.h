#pragma once
#include <thread>
#include <functional>

namespace QFE {

	class MultiThreadRunFunction final {
	public:
		/// isRunningはfalseで初期匁E
		MultiThreadRunFunction();
		/// 破棁Eに別スレチEで実行してぁE物があれE終亁Eで征E
		~MultiThreadRunFunction();

		/// 別スレチEで実行してぁE物があれE終亁Eで征E初期匁E
		void Init();
		/// 関数を別スレチEで実行すめE
		void Start(std::function<void()> func);
		/// 開始したか
		const bool IsStarted() const;
		/// 実行中ぁE
		const bool IsRunning() const;
		/// 終わってぁEぁE
		const bool IsSuccess() const;

	private:
		std::thread t;
		bool isRunning;
		bool isStarted;
	};

}

#pragma once
#include <thread>
#include <functional>

class MultiThreadRunFunction final {
public:
	/// isRunningはfalseで初期化
	MultiThreadRunFunction();
	/// 破棄時に別スレッドで実行中の処理があれば終了まで待つ
	~MultiThreadRunFunction();

	/// 別スレッドで実行中の処理があれば終了まで待って初期化
	void Init();
	/// 関数を別スレッドで実行する
	void Start(std::function<void()> func);
	/// 開始したか
	const bool IsStarted() const;
	/// 実行中い
	const bool IsRunning() const;
	/// 終わっているい
	const bool IsSuccess() const;

private:
	std::thread t;
	bool isRunning;
	bool isStarted;
};

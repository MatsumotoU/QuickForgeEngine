#pragma once
#include <thread>
#include <functional>

class MultiThreadRunFunction final {
public:
	/// isRunningはfalseで初期化
	MultiThreadRunFunction();
	/// 破棄時に別スレッドで実行している物があれば終了まで待つ
	~MultiThreadRunFunction();

	/// 別スレッドで実行している物があれば終了まで待つ初期化
	void Init();
	/// 関数を別スレッドで実行する
	void Start(std::function<void()> func);
	/// 開始したか
	const bool IsStarted() const;
	/// 実行中か
	const bool IsRunning() const;
	/// 終わっているか
	const bool IsSuccess() const;

private:
	std::thread t;
	bool isRunning;
	bool isStarted;
};
#pragma once
#include <thread>
#include <functional>

class MultiThreadRunFunction final {
public:
	/// isRunningはfalseで初期匁E
	MultiThreadRunFunction();
	/// 破棁E��に別スレチE��で実行してぁE��物があれ�E終亁E��で征E��
	~MultiThreadRunFunction();

	/// 別スレチE��で実行してぁE��物があれ�E終亁E��で征E��初期匁E
	void Init();
	/// 関数を別スレチE��で実行すめE
	void Start(std::function<void()> func);
	/// 開始したか
	const bool IsStarted() const;
	/// 実行中ぁE
	const bool IsRunning() const;
	/// 終わってぁE��ぁE
	const bool IsSuccess() const;

private:
	std::thread t;
	bool isRunning;
	bool isStarted;
};

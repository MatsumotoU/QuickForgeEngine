#pragma once
#include <thread>
#include <functional>

class MultiThreadRunFunction final {
public:
	/// isRunning縺ｯfalse縺ｧ蛻晄悄蛹・
	MultiThreadRunFunction();
	/// 遐ｴ譽・凾縺ｫ蛻･繧ｹ繝ｬ繝・ラ縺ｧ螳溯｡後＠縺ｦ縺・ｋ迚ｩ縺後≠繧後・邨ゆｺ・∪縺ｧ蠕・▽
	~MultiThreadRunFunction();

	/// 蛻･繧ｹ繝ｬ繝・ラ縺ｧ螳溯｡後＠縺ｦ縺・ｋ迚ｩ縺後≠繧後・邨ゆｺ・∪縺ｧ蠕・▽蛻晄悄蛹・
	void Init();
	/// 髢｢謨ｰ繧貞挨繧ｹ繝ｬ繝・ラ縺ｧ螳溯｡後☆繧・
	void Start(std::function<void()> func);
	/// 髢句ｧ九＠縺溘°
	const bool IsStarted() const;
	/// 螳溯｡御ｸｭ縺・
	const bool IsRunning() const;
	/// 邨ゅｏ縺｣縺ｦ縺・ｋ縺・
	const bool IsSuccess() const;

private:
	std::thread t;
	bool isRunning;
	bool isStarted;
};

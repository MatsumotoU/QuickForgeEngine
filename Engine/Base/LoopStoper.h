#pragma once
#include <functional>
#include <vector>

class LoopStoper {
public:
	void Initialize();
	void Update();

public:
	void AddNonStoppingFunc(std::function<void()> func);
	bool GetIsStopping();
	void SetIsStopping(bool set);

private:
	bool isStopping_;
	std::vector<std::function<void()>> nonTimeStoppingFunctions_;
};
#pragma once
#include <string>
class Enemy;

class BaseEnemyState {
public:
	BaseEnemyState(const std::string& name, Enemy* enemy) : name_(name), enemy_(enemy) {};
	virtual ~BaseEnemyState() = default;
	virtual void Update() = 0;

protected:
	std::string name_;
	Enemy* enemy_ = nullptr;
};
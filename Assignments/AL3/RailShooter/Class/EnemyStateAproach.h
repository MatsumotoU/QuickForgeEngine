#pragma once
#include "BaseEnemyState.h"

class EnemyStateAproach : public BaseEnemyState {
public:
	EnemyStateAproach(Enemy* enemy);
	~EnemyStateAproach()override {};
	void Update()override;
};
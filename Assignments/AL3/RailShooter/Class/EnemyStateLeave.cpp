#include "EnemyStateLeave.h"
#include "Enemy.h"

EnemyStateLeave::EnemyStateLeave(Enemy* enemy) 
	:BaseEnemyState("State Leave", enemy) {}

void EnemyStateLeave::Update() {
	enemy_->Leave();
}

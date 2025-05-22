#include "EnemyStateAproach.h"
#include "Enemy.h"

EnemyStateAproach::EnemyStateAproach(Enemy* enemy)
	:BaseEnemyState("State Aproach",enemy) {}

void EnemyStateAproach::Update() {
	enemy_->Approch();

	/*if (enemy_->GetPhase() == Phase::Leave) {
		enemy_->ChangeState(std::make_unique<EnemyStateLeave>(this));
	}*/
}
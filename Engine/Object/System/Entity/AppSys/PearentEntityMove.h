#pragma once

class EntityManager;

class ParentEntityMove {
public:
	ParentEntityMove() = default;
	~ParentEntityMove() = default;
	// 親エンティティの位置を子エンティティに適用する
	static void ParentUpdate(EntityManager& entityManager);
};
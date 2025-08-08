#pragma once
#include <string>
class EngineCore;
class EntityManager;

class EntityLoader{
public:
	static void LoadEntities(EngineCore* engineCore, EntityManager& entityManager, const std::string& fileName);
	static void SaveEntities(EngineCore* engineCore, EntityManager& entityManager, const std::string& fileName);
};
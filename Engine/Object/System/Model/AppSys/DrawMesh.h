#pragma once

class EngineCore;
class Camera;
class EntityManager;

class DrawMesh {
public:
	static void Draw(EngineCore* engineCore, EntityManager& entity, Camera& camera);
};
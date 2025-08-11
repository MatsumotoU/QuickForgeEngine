#pragma once
struct LuaScriptComponent;
class EngineCore;
class EntityManager;

class EntityScriptUpdate final {
	/// <summary>
	/// 指定のLuaスクリプトのUpdate関数を実行します。
	/// </summary>
	/// <param name="engineCore"></param>
	/// <param name="entityManager"></param>
	/// <param name="scriptComponent"></param>
	static void Update(EngineCore* engineCore, EntityManager& entityManager);
};
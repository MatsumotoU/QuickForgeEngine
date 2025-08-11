#pragma once
#include <string>
class EngineCore;
class EntityManager;

class AttachScriptForEntity {
public:
	/// <summary>
	/// 指定のエンティティにLuaスクリプトをアタッチします。
	/// </summary>
	/// <param name="engineCore">エンジンコア</param>
	/// <param name="entityManager">エンティティマネージャー</param>
	/// <param name="entityId">エンティティID</param>
	/// <param name="scriptName">Luaスクリプトの名前</param>
	static void Attach(EngineCore* engineCore, EntityManager& entityManager, uint32_t entityId, const std::string& scriptName);
};
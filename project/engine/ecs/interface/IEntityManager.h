#pragma once
#include <cstdint>

namespace QFE{
	/// @brief エンティティマネージャのインターフェース。エンティティの作成、削除、コンポーネントの取得・削除を行う。
	class IEntityManager
	{
		public:
			virtual ~IEntityManager() = default;
			/// @brief 新しいエンティティを作成する。新しいエンティティIDを返す。
			virtual uint32_t CreateEntity() = 0;
			/// @brief エンティティを削除予定リストに追加する。フレーム終了時に削除される。
			virtual void RemoveEntity(uint32_t id) = 0;
			/// @brief コンポーネントの生ポインタを取得する。存在しない場合はnullptrを返す。
			virtual void* GetComponentRaw(uint32_t entityId, const char* componentTypeName) = 0;
			/// @brief エンティティのコンポーネントを削除する。存在しない場合は何もしない。
			virtual void RemoveComponent(uint32_t entityId, const char* componentTypeName) = 0;
	};
}

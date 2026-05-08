#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <vector>
#include <string>
#include "engine/include/utility/memory/SparseSets.h"

namespace QFE {

	class EntityManager;

	/**
	 * @brief シーン固有のC#スクリプト実行環境
	 * 各シーンが独自のAppDomainとスクリプトインスタンスを持つ
	 */
	class CsharpScriptExecutor {
	public:
		CsharpScriptExecutor();
		~CsharpScriptExecutor();

		/// @brief 初期化（EntityManagerへの参照を設定し、AppDomainを作成）
		void Initialize(EntityManager* entityManager);

		/// @brief 全スクリプトをリセット
		void ResetScripts();

		/// @brief 終了処理（AppDomainをアンロード）
		void Finalize();

		/// @brief スクリプトインスタンスを作成
		/// @param entityId バインドするエンティティID
		/// @param className クラス名（名前空間含む）
		/// @return スクリプトインスタンスのインデックス
		uint32_t CreateScriptInstance(uint32_t entityId, const std::string& className);

		/// @brief スクリプトインスタンスを直接生成
		/// @param className クラス名(名前空間含む)
		void CreateScriptInstance(const std::string& className);

		/// @brief スクリプトインスタンスを削除
		/// @param index スクリプトインスタンスのインデックス
		void DeleteScriptInstance(uint32_t index);

		/// @brief スクリプト関数を実行
		/// @param index スクリプトインスタンスのインデックス
		/// @param functionName 関数名
		void RunScriptFunction(uint32_t index, const std::string& functionName);

		/// @brief 全スクリプトの指定関数を実行
		/// @param functionName 関数名
		void RunAllScriptsFunction(const std::string& functionName);

		/// @brief 利用可能なクラス名を取得
		/// @return クラス名のリスト
		std::vector<std::string> GetAvailableScriptClasses() const;

		/// @brief スクリプト数を取得
		int GetScriptCount() const { return static_cast<int>(scriptInstances_.size()); }

		/// @brief アセンブリをリロード
		void ReloadAssembly();

	private:
		EntityManager* entityManager_ = nullptr;
		MonoDomain* domain_ = nullptr;  // このシーン専用のAppDomain
		MonoAssembly* assembly_ = nullptr;
		SparseSet<MonoObject*> scriptInstances_; 

		/// @brief アセンブリをロード
		void LoadAssembly();
	};

}

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

		/// @brief 全スクリプトをリセット
		void ResetScripts();

		/// @brief 初期化（EntityManagerへの参照を設定し、AppDomainを作成）
		void Initialize(EntityManager* entityManager);

		/// @brief ゲームロジックの初期化（GameLogicManagerのInitializeを呼び出す）
		void InitializeGameLogic(EntityManager* entityManager);
		/// @brief 毎フレームの開始処理（GameLogicManagerのFrameStartを呼び出す）
		void FrameStart();
		/// @brief 毎フレームの更新処理（GameLogicManagerのUpdateを呼び出す）
		void Update();
		/// @brief 毎フレームの終了処理（GameLogicManagerのFrameEndを呼び出す）
		void FrameEnd();

		/// @brief 終了処理（AppDomainをアンロード）
		void Finalize();

		/// @brief スクリプトインスタンスを作成
		/// @param entityId バインドするエンティティID
		/// @param className クラス名（名前空間含む）
		/// @return スクリプトインスタンスのインデックス
		uint32_t CreateScriptInstance(uint32_t entityId, const std::string& className);

		/// @brief スクリプトインスタンスを削除
		/// @param index スクリプトインスタンスのインデックス
		void DeleteScriptInstance(uint32_t index);

		/// @brief 利用可能なクラス名を取得
		/// @return クラス名のリスト
		std::vector<std::string> GetAvailableScriptClasses() const;

		/// @brief スクリプト数を取得
		int GetScriptCount() const { return 1; }

		/// @brief アセンブリをリロード
		void ReloadAssembly();
		/// @brief ゲームロジックマネージャーをリセット
		void ResetGameLogicManager();

	private:
		EntityManager* entityManager_ = nullptr;
		MonoDomain* domain_ = nullptr;  // このシーン専用のAppDomain
		MonoAssembly* assembly_ = nullptr;

		MonoClass* gameLogicManagerClass_ = nullptr; // GameLogicManagerクラス
		MonoObject* gameLogicManagerInstance_ = nullptr; // GameLogicManagerのインスタンス
		MonoMethod* gameLogicInitializeMethod_ = nullptr; // GameLogicManagerのInitializeメソッド
		MonoMethod* gameLogicUpdateMethod_ = nullptr; // GameLogicManagerのUpdateメソッド
		MonoMethod* gameLogicFrameStartMethod_ = nullptr; // GameLogicManagerのFrameStartメソッド
		MonoMethod* gameLogicFrameEndMethod_ = nullptr; // GameLogicManagerのFrameEndメソッド
		MonoMethod* gameLogicCreateScriptInstanceMethod_ = nullptr; // GameLogicManagerのCreateScriptInstanceメソッド

		/// @brief アセンブリをロード
		void LoadAssembly();
	};

}

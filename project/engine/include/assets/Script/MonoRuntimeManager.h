#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string>

namespace QFE {

	/**
	 * @brief Monoランタイムのグローバル管理
	 * プロセスごとに1回のみ初期化される
	 * C# APIの登録とJIT初期化を担当
	 */
	class MonoRuntimeManager final : public Singleton<MonoRuntimeManager> {
		friend class Singleton<MonoRuntimeManager>;

	public:
		/// @brief Mono JITの初期化とディレクトリ設定
		void Initialize();

		/// @brief Mono JITのクリーンアップ
		void Finalize();

		/// @brief QFE C# APIの登録（グローバルに1回のみ）
		void RegisterQFEAPI();

		/// @brief C#スクリプトのプロジェクトを生成してコンパイル
		void CompileScripts();

		/// @brief C#プロジェクトの生成
		void CreateCSProject(const std::string& projectName);

		/// @brief ルートドメインの取得
		MonoDomain* GetRootDomain() const { return rootDomain_; }

		/// @brief アセンブリパスの取得
		std::string GetAssemblyPath() const;

		/// @brief 初期化済みかどうか
		bool IsInitialized() const { return rootDomain_ != nullptr; }

	private:
		MonoRuntimeManager() = default;
		~MonoRuntimeManager() = default;
		MonoRuntimeManager(const MonoRuntimeManager&) = delete;
		MonoRuntimeManager& operator=(const MonoRuntimeManager&) = delete;

		MonoDomain* rootDomain_ = nullptr;
	};

}

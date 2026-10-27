#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

namespace QFE::INPUT {
	class InputInterface;
}

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::FRAMEWORK {
	/// @brief ScriptComponentを持つEntityのLuaスクリプトを管理する。
	/// @details BoomブランチのLuaScriptResourceManagerを現行ECS向けに移植したもの。
	class LuaScriptSystem final {
	public:
		LuaScriptSystem();
		~LuaScriptSystem();

		LuaScriptSystem(const LuaScriptSystem&) = delete;
		LuaScriptSystem& operator=(const LuaScriptSystem&) = delete;
		LuaScriptSystem(LuaScriptSystem&&) noexcept;
		LuaScriptSystem& operator=(LuaScriptSystem&&) noexcept;

		/// @param scriptRoot Luaファイルを探索する基準ディレクトリ。
		void Initialize(std::filesystem::path scriptRoot = "resources/Scripts");
		/// @brief Componentの追加・削除・変更を同期し、Init/Updateを呼び出す。
		void Update(
			QFE::SCENE::SceneManager& sceneManager,
			QFE::INPUT::InputInterface& input,
			float deltaTime);
		/// @brief 現在ロードしている全スクリプトを次回Updateで再ロードする。
		void RequestReloadAll();
		/// @brief 全Lua状態を破棄する。Scene/DLLのリロード前にも使用する。
		void Reset();
		void Shutdown();

		[[nodiscard]] std::size_t GetLoadedScriptCount() const;

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};
}
